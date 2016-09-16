#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>

#include "base/base.h"
#include "nineserver/sql/mysql.h"

DEFINE_string(query, "SELECT 1", "Query.");

int main(int argc, char **argv) {
  ParseCommandLineFlags(&argc, &argv);
  CHECK(MySQL::Connect() != nullptr);
  string query;
  for (int i = 1; i < argc; i++) {
    if (i != 1) { query.push_back(' '); }
    query.append(argv[i]);
  }
  QueryResult query_result = MySQL::SelectAll(query);
  if (query_result.arena_buffer != nullptr) {
    LOG(INFO) << "Usage: " << query_result.arena_buffer->usage();
  }
  LOG(INFO) << "Num rows: " << query_result.num_rows;
  LOG(INFO) << "Num fields: " << query_result.num_fields;
  printf("(");
  for (int x = 0; x < query_result.num_fields; x++) {
    if (x != 0) { printf(", "); }
    printf("`%s`", query_result.Field(x).ToString().c_str());
  }
  printf(") VALUES\n");
  for (int y = 0; y < query_result.num_rows; y++) {
    if (y != 0) { puts(", "); }
    printf("(");
    for (int x = 0; x < query_result.num_fields; x++) {
      if (x != 0) { printf(", "); }
      if (QueryResult::IsNull(query_result.Get(y, x))) {
        printf("NULL");
      } else {
        printf("\"%s\"", MySQL::Escape(query_result.Get(y, x)).c_str());
      }
    }
    printf(")");
  }
  puts("");
}
