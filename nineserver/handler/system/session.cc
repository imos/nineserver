#include "nineserver/handler/logging.h"
#include "nineserver/handler/system/system.h"
#include "nineserver/http/html.h"

namespace {

bool SessionHandler(BaseHandler* handler) {
  SystemHandlerTemplate handler_template("Session Information", handler);
  handler_template.Header("Session Information");
  handler->Print(
      R"(<h3>Update session</h3>)");

  if (handler->ENV("REQUEST_METHOD"_a) == "POST"_a) {
    ArenaStringPiece data = handler->POST("data"_a);
    string error;
    Json new_session = Json::parse(data.ToString(), error);
    if (!error.empty()) {
      handler->Print(
          R"(<div class="alert alert-danger"><strong>Syntax Error!</strong> )",
          EscapeHtml(error), "</div>");
    } else {
      handler->SetSessionData(new_session);
      handler->Print(
          R"(<div class="alert alert-success"><strong>Success!</strong> )",
          "Session was successfully updated.</div>");
    }
  }

  handler->Print(
      R"(<form action="#" method="POST">)"
      R"(<textarea name="data" )"
      R"(style="width:100%;height:200px;font-family:monospace;">)",
      EscapeHtml(JsonEncode(handler->SESSION(), true)),
      R"(</textarea><br><input type="submit" value="Update"></form>)");
  handler->Print(
      R"(<h3>Current Session data</h3><pre class="prettyprint lang-js">)"_a,
      EscapeHtml(JsonEncode(handler->SESSION(), true)), "</pre>"_a);

  return true;
}
REGISTER_HANDLER(
    SessionHandler, SYSTEM_HANDLER_PREFIX "/session$", SessionHandler);

}  // namespace
