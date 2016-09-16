#include "nineserver/handler/logging.h"
#include "nineserver/handler/system/system.h"
#include "nineserver/http/html.h"
#include "third_party/gflags/gflags.h"

bool InfoHandler(BaseHandler* handler) {
  SystemHandlerTemplate handler_template("Request Information", handler);
  handler_template.Header("Request Information");
  handler->Print(R"(<details><summary>Form</summary>)"_a);
  handler->Print(R"(<form action="#" method="GET">GET: )"
                 R"(<input type="text" name="name" value="value">)"
                 R"(<input type="submit" value="Submit"></form>)");
  handler->Print(R"(<form action="#" method="POST">POST: )"
                 R"(<input type="text" name="name" value="value">)"
                 R"(<input type="submit" value="Submit"></form>)");
  handler->Print(R"(<form enctype="multipart/form-data" )"
                 R"(action="#" method="POST">FILE: )"
                 R"(<input type="file" name="name">)"
                 R"(<input type="submit" value="Submit"></form>)");
  handler->Print("</details>"_a);
  handler->Print(R"(<h3>GET</h3><pre class="prettyprint lang-js">)"_a,
                 EscapeHtml(JsonEncode(handler->GET(), true)), "</pre>"_a);
  handler->Print(R"(<h3>POST</h3><pre class="prettyprint lang-js">)"_a,
                 EscapeHtml(JsonEncode(handler->POST(), true)), "</pre>"_a);
  handler->Print(R"(<h3>ENV</h3><pre class="prettyprint lang-js">)"_a,
                 EscapeHtml(JsonEncode(handler->ENV(), true)), "</pre>"_a);
  handler->Print(R"(<h3>FILE</h3><pre class="prettyprint lang-js">)"_a,
                 EscapeHtml(JsonEncode(handler->FILE(), true)), "</pre>"_a);
  handler->Print(R"(<h3>COOKIE</h3><pre class="prettyprint lang-js">)"_a,
                 EscapeHtml(JsonEncode(handler->COOKIE(), true)), "</pre>"_a);
  handler->Print(R"(<h3>SESSION</h3><pre class="prettyprint lang-js">)"_a,
                 EscapeHtml(JsonEncode(handler->SESSION(), true)), "</pre>"_a);
  handler->Print(R"(<h3>Request</h3><pre class="prettyprint lang-js">)"_a,
                 EscapeHtml(JsonEncode(*handler->MutableRequest(), true)),
                 "</pre>"_a);
  handler->Print(R"(<h3>Response</h3><pre class="prettyprint lang-js">)"_a,
                 EscapeHtml(JsonEncode(*handler->MutableResponse(), true)),
                 "</pre>"_a);

  vector<google::CommandLineFlagInfo> flags;
  google::GetAllFlags(&flags);
  handler->Print(R"(<h3>Flags</h3><ul>)");
  for (const auto& flag : flags) {
    if (HasPrefixString(flag.filename, "external/")) continue;
    handler->Print("<li>--", EscapeHtml(flag.name), "="_a,
                   flag.current_value != flag.default_value
                      ? "<b>" + EscapeHtml(flag.current_value) + "</b>"
                      : EscapeHtml(flag.current_value),
                   " <span style=\"color:#888\">("_a,
                   EscapeHtml(flag.filename),
                   ")</span><blockquote>"_a,
                   EscapeHtml(flag.description), "</blockquote></li>"_a);
  }
  handler->Print("</ul>");

  return true;
}
REGISTER_HANDLER(InfoHandler, SYSTEM_HANDLER_PREFIX "/$", InfoHandler);
