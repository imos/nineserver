#include "nineserver/sql/mysql.h"

#include <time.h>

#include "base/base.h"

DEFINE_string(mysql_host, "localhost", "MySQL's host name to connect.");
DEFINE_string(mysql_user, "root", "MySQL user.");
DEFINE_string(mysql_password, "", "MySQL password.");
DEFINE_string(mysql_database, "mysql", "Database to select.");
DEFINE_int32(mysql_port, 3306, "Port number to connect.");
DEFINE_string(mysql_socket, "", "UNIX domain socket.");

string ToSqlTimestamp(time_t time) {
  if (time < 0) {
    return "0000-00-00 00:00:00";
  }
  char buffer[20];
  struct tm tm;
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S",
           localtime_r(&time, &tm));
  return buffer;
}

time_t FromSqlTimestamp(StringPiece sql_timestamp) {
  if (sql_timestamp.size() != "0000-00-00 00:00:00"_a.size()) {
    return -1;
  }
  if (!isdigit(sql_timestamp[0]) || !isdigit(sql_timestamp[1]) ||
      !isdigit(sql_timestamp[2]) || !isdigit(sql_timestamp[3]) ||
      sql_timestamp[4] != '-' ||
      !isdigit(sql_timestamp[5]) || !isdigit(sql_timestamp[6]) ||
      sql_timestamp[7] != '-' ||
      !isdigit(sql_timestamp[8]) || !isdigit(sql_timestamp[9]) ||
      sql_timestamp[10] != ' ' ||
      !isdigit(sql_timestamp[11]) || !isdigit(sql_timestamp[12]) ||
      sql_timestamp[13] != ':' ||
      !isdigit(sql_timestamp[14]) || !isdigit(sql_timestamp[15]) ||
      sql_timestamp[16] != ':' ||
      !isdigit(sql_timestamp[17]) || !isdigit(sql_timestamp[18])) {
    return -1;
  }
  struct tm t;
  t.tm_year = (sql_timestamp[0] & 15) * 1000 +
              (sql_timestamp[1] & 15) * 100 +
              (sql_timestamp[2] & 15) * 10 +
              (sql_timestamp[3] & 15) - 1900;
  t.tm_mon = (sql_timestamp[5] & 15) * 10 + (sql_timestamp[6] & 15) - 1;
  t.tm_mday = (sql_timestamp[8] & 15) * 10 + (sql_timestamp[9] & 15);
  t.tm_hour = (sql_timestamp[11] & 15) * 10 + (sql_timestamp[12] & 15);
  t.tm_min = (sql_timestamp[14] & 15) * 10 + (sql_timestamp[15] & 15);
  t.tm_sec = (sql_timestamp[17] & 15) * 10 + (sql_timestamp[18] & 15);
  t.tm_wday = 0;
  t.tm_yday = 0;
  t.tm_isdst = 0;
  return mktime(&t);
}

Json QueryResult::ToJson() const {
  vector<Json> json;
  {
    vector<Json> fields;
    for (int field_index = 0; field_index < num_fields; field_index++) {
      fields.push_back(Json(Field(field_index)));
    }
    json.push_back(Json(std::move(fields)));
  }
  for (int row_index = 0; row_index < num_rows; row_index++) {
    vector<Json> row;
    for (int field_index = 0; field_index < num_fields; field_index++) {
      ArenaStringPiece cell = Get(row_index, field_index);
      if (IsNull(cell)) {
        row.push_back(Json(nullptr));
      } else {
        row.push_back(Json(cell));
      }
    }
    json.push_back(Json(row));
  }
  return json;
}

ArenaStringPiece QueryResult::Null() {
  static const char* mysql_null = "\0MYSQL_NULL";
  return ArenaStringPiece(StringPiece(mysql_null, 0));
}

bool QueryResult::IsNull(StringPiece value) {
  return Null().data() == value.data();
}

MYSQL* MySQL::Connect() {
  thread_local MYSQL* connection = nullptr;
  if (connection == nullptr) {
    connection = mysql_init(nullptr);
    if (!mysql_real_connect(
        connection, FLAGS_mysql_host.c_str(), FLAGS_mysql_user.c_str(),
        FLAGS_mysql_password.empty() ? nullptr : FLAGS_mysql_password.c_str(),
        FLAGS_mysql_database.c_str(), FLAGS_mysql_port,
        FLAGS_mysql_socket.empty() ? nullptr : FLAGS_mysql_socket.c_str(), 0)) {
      LOG_FIRST_N(ERROR, 10)
          << "Failed to connect SQL server: " << mysql_error(connection);
      connection = nullptr;
    }
    if (mysql_set_character_set(connection, "utf8") != 0) {
      LOG_FIRST_N(ERROR, 10) << "Failed to set character set.";
    }
  }
  return connection;
}

bool MySQL::Command(StringPiece query) {
  MYSQL* connection = Query(query);
  if (connection == nullptr) { return false; }
  MYSQL_RES* result = mysql_store_result(connection);
  if (result != nullptr) { mysql_free_result(result); }
  return true;
}

QueryResult MySQL::SelectAll(StringPiece query, ArenaBuffer* arena_buffer) {
  MYSQL* connection = Query(query);
  if (connection == nullptr) { return QueryResult{}; }
  MYSQL_RES* result = mysql_store_result(connection);
  if (result == nullptr) { return QueryResult{}; }
  QueryResult query_result;
  query_result.num_rows = mysql_num_rows(result);
  query_result.num_fields = mysql_num_fields(result);
  if (arena_buffer == nullptr) {
    query_result.arena_buffer.reset(new ArenaBuffer);
    arena_buffer = query_result.arena_buffer.get();
  }
  query_result.data.reset(new ArenaStringPiece[
      static_cast<int64>(query_result.num_rows + 1) * query_result.num_fields]);
  MYSQL_FIELD* fields = mysql_fetch_fields(result);
  for (int field_id = 0; field_id < query_result.num_fields; field_id++) {
    arena_buffer->Init();
    arena_buffer->append(StringPiece(fields[field_id].name));
    query_result.data[field_id] = *arena_buffer;
  }
  for (int row_id = 0; row_id < query_result.num_rows; row_id++) {
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row == nullptr) {
      query_result.num_rows = row_id;
      break;
    }
    auto* lengths = mysql_fetch_lengths(result);
    for (int field_id = 0; field_id < query_result.num_fields; field_id++) {
      arena_buffer->Init();
      arena_buffer->resize(lengths[field_id]);
      memcpy(arena_buffer->data(), row[field_id], arena_buffer->size());
      query_result.data[(row_id + 1) * query_result.num_fields + field_id] =
          row[field_id] == nullptr ? QueryResult::Null() : *arena_buffer;
    }
  }
  mysql_free_result(result);
  return query_result;
}

MYSQL* MySQL::Query(StringPiece query) {
  MYSQL* connection = Connect();
  if (connection == nullptr) { return nullptr; }
  if (mysql_real_query(connection, query.data(), query.size()) != 0) {
    LOG(WARNING) << mysql_error(connection) << ": " << query;
    return nullptr;
  }
  return connection;
}

int64 MySQL::GetAffectedRows() {
  MYSQL* connection = Connect();
  if (connection == nullptr) { return -1; }
  return static_cast<int64>(mysql_affected_rows(connection));
}

int64 MySQL::GetInsertId() {
  MYSQL* connection = Connect();
  if (connection == nullptr) { return -1; }
  return static_cast<int64>(mysql_insert_id(connection));
}

bool MySQL::Escape(StringPiece data, BufferInterface* buffer) {
  MYSQL* connection = Connect();
  if (connection == nullptr) { return false; }
  char output[data.size() * 2 + 1];
  int64 length = mysql_real_escape_string(
      connection, output, data.data(), data.size());
  buffer->append(StringPiece(output, length));
  return true;
}

string MySQL::Escape(StringPiece data) {
  string result;
  StringBuffer buffer(&result);
  Escape(data, &buffer);
  return result;
}
