#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>

#include <memory>

#include "base/base.h"
#include "nineserver/arena/arena_buffer.h"
#include "nineserver/json.h"

string ToSqlTimestamp(time_t time);
time_t FromSqlTimestamp(StringPiece sql_timestamp);

struct QueryResult {
  int32 num_rows = 0;
  int32 num_fields = 0;
  std::unique_ptr<ArenaStringPiece[]> data;
  std::unique_ptr<ArenaBuffer> arena_buffer;

  struct CellIterator {
    const QueryResult* result = nullptr;
    int64 row_index = 0;
    int64 cell_index = 0;

    ArenaStringPiece operator*() { return result->Get(row_index, cell_index); }
    CellIterator& operator++() { cell_index++; return *this; }
    CellIterator& operator++(int) { return ++*this; }
    bool operator==(const CellIterator& rhs) const {
      return result == rhs.result && row_index == rhs.row_index &&
             cell_index == rhs.cell_index;
    }
    bool operator!=(const CellIterator& rhs) const { return !(*this == rhs); }
  };

  struct RowIterator {
    const QueryResult* result = nullptr;
    int64 row_index = 0;

    typedef CellIterator iterator;
    iterator begin() const {
      iterator it;
      it.result = result;
      it.row_index = row_index;
      it.cell_index = 0;
      return it;
    }

    iterator end() const {
      iterator it = begin();
      it.cell_index = result->num_fields;
      return it;
    }

    ArenaStringPiece operator[](int64 column_index) const {
      return result->Get(row_index, column_index);
    }

    size_t size() const { return result->num_fields; }

    RowIterator& operator*() { return *this; }

    RowIterator& operator++() { row_index++; return *this; }
    RowIterator& operator++(int) { return ++*this; }
    bool operator==(const RowIterator& rhs) const
        { return result == rhs.result && row_index == rhs.row_index; }
    bool operator!=(const RowIterator& rhs) const { return !(*this == rhs); }
  };

  typedef RowIterator iterator;

  iterator begin() const {
    iterator it;
    it.result = this;
    it.row_index = 0;
    return it;
  }

  iterator end() const {
    iterator it = begin();
    it.row_index = num_rows;
    return it;
  }

  RowIterator operator[](int64 row_index) const {
    iterator it = begin();
    it.row_index = row_index;
    return it;
  }

  size_t size() const { return num_rows; }

  inline ArenaStringPiece Field(int field_index) const {
    DCHECK_GE(field_index, 0);
    DCHECK_LT(field_index, num_fields);
    return data[field_index];
  }

  inline ArenaStringPiece Get(int row_index, int field_index) const {
    DCHECK_GE(row_index, 0);
    DCHECK_LT(row_index, num_rows);
    DCHECK_GE(field_index, 0);
    DCHECK_LT(field_index, num_fields);
    return data[(row_index + 1) * num_fields + field_index];
  }

  Json ToJson() const;

  static ArenaStringPiece Null();
  static bool IsNull(StringPiece value);
};

class MySQL {
 public:
  static MYSQL* Connect();

  static bool Command(StringPiece query);

  static QueryResult SelectAll(
      StringPiece query, ArenaBuffer* arena_buffer = nullptr);

  static int64 GetAffectedRows();
  static int64 GetInsertId();

  static bool Escape(StringPiece data, BufferInterface* buffer);
  static string Escape(StringPiece data);

 private:
  static MYSQL* Query(StringPiece query);
};
