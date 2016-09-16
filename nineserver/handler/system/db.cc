#include "nineserver/handler/system/system.h"
#include "nineserver/http/html.h"
#include "nineserver/sql/mysql.h"

namespace {

void ShowInnoDBStatus(BaseHandler* handler) {
  QueryResult result = MySQL::SelectAll("SHOW ENGINE INNODB STATUS");
  handler->Print(R"(<h3>InnoDB Status</h3>)");
  for (int row_index = 0; row_index < result.num_rows; row_index++) {
    for (int field_id = 0; field_id < result.num_fields; field_id++) {
      if (result.Field(field_id) != "Status"_a) continue;
      handler->Print(R"(<pre class="prettyprint lang-js">)",
                     EscapeHtml(result.Get(row_index, field_id)), R"(</pre>)");
    }
  }
}

void ShowVariables(BaseHandler* handler) {
  QueryResult result = MySQL::SelectAll("SHOW VARIABLES");
  handler->Print(R"(<h3>Variables</h3>)");
  handler->Print(
      R"(<div class="table-responsive"><table class="table table-striped">)");
  handler->Print("<thead><tr><th>Key</th><th>Value</th></tr></thead>");
  for (int row_index = 0; row_index < result.num_rows; row_index++) {
    handler->Print("<tr>");
    for (int field_id = 0; field_id < result.num_fields; field_id++) {
      handler->Print(
          "<td>", EscapeHtml(result.Get(row_index, field_id)), "</td>");
    }
    handler->Print("</tr>");
  }
  handler->Print("</table></div>");
}

void ShowStatus(BaseHandler* handler) {
  QueryResult result = MySQL::SelectAll("SHOW STATUS");
  handler->Print(R"(<h3>Status</h3>)");
  handler->Print(
      R"(<div class="table-responsive"><table class="table table-striped">)");
  handler->Print("<thead><tr><th>Key</th><th>Value</th></tr></thead>");
  for (int row_index = 0; row_index < result.num_rows; row_index++) {
    handler->Print("<tr>");
    for (int field_id = 0; field_id < result.num_fields; field_id++) {
      handler->Print(
          "<td>", EscapeHtml(result.Get(row_index, field_id)), "</td>");
    }
    handler->Print("</tr>");
  }
  handler->Print("</table></div>");
}

bool DatabaseHandler(BaseHandler* handler) {
  SystemHandlerTemplate handler_template("Database Status", handler);
  handler_template.Header("Database Status");
  ShowInnoDBStatus(handler);
  ShowVariables(handler);
  ShowStatus(handler);
  return true;
}
REGISTER_HANDLER(
    DatabaseHandler, SYSTEM_HANDLER_PREFIX "/db$", DatabaseHandler);

}  // namespace
