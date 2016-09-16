#include "nineserver/handler/logging.h"
#include "nineserver/handler/system/system.h"
#include "nineserver/http/html.h"

void PrintTable(BaseHandler* handler, bool per_request) {
  const auto stats = Logging::GetStats();
  auto fields = Logging::GetFields();
  string key = handler->GET("key"_a).ToString();

  handler->Print(
      R"(<div class="table-responsive"><table class="table table-striped">)");
  handler->Print("<thead><tr><th><a href=\"?key=path\">Path</a></th>");
  handler->Print("<th><a href=\"?\"># of requests</a></th>");
  for (const auto& field : fields) {
    handler->Print("<th><a href=\"?key=", string(field.key), "\">",
                   EscapeHtml(field.name), "</a></th>");
  }
  handler->Print("</tr></thead>");
  vector<pair<int64, ArenaStringPiece>> rows;
  for (const auto& path_and_key_to_value : stats) {
    ArenaBuffer* buffer = handler->NextBuffer();
    const auto& data = path_and_key_to_value.second;
    int64 num_requests = FindWithDefault(data, "", -1);
    buffer->StrCat("<tr><td>"_a, EscapeHtml(path_and_key_to_value.first),
                   "</td><td>"_a, num_requests, "</td>"_a);
    for (const auto& field : fields) {
      int64 value = FindWithDefault(data, field.key, -1);
      if (value >= 0 && per_request) {
        value /= num_requests;
      }
      if (value < 0) {
        buffer->StrCat("<td>N/A</td>"_a);
      } else {
        value = (value + field.divisor / 2) / field.divisor;
        buffer->StrCat(
            "<td>"_a, value, " "_a, EscapeHtml(field.unit), "</td>"_a);
      }
    }
    buffer->StrCat("</tr>"_a);
    int64 value = FindWithDefault(data, key, 0);
    if (value >= 0 && per_request && key != ""_a) {
      value /= num_requests;
    }
    rows.push_back(pair<int64, ArenaStringPiece>(
        -value, buffer->ToArenaStringPiece()));
  }
  sort(rows.begin(), rows.end());
  for (const auto& row : rows) { handler->Print(row.second); }
  handler->Print("</table></div>");
}

bool StatHandler(BaseHandler* handler) {
  SystemHandlerTemplate handler_template("Access Statistics", handler);
  handler_template.Header("Access Statistics");

  handler->Print("<h3>Total resource</h3>");
  PrintTable(handler, false /* per_request */);
  handler->Print("<h3>Resource per request</h3>");
  PrintTable(handler, true /* per_request */);

  return true;
}
REGISTER_HANDLER(StatHandler, SYSTEM_HANDLER_PREFIX "/stats$", StatHandler);
