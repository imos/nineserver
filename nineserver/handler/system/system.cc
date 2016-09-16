#include "nineserver/handler/system/system.h"

SystemHandlerTemplate::SystemHandlerTemplate(
    StringPiece title, BaseHandler* handler)
    : handler_(handler) {
  handler->Print(
      R"(<!DOCTYPE html>)"
      R"(<html lang="en">)"
      R"(  <head>)"
      R"(    <meta charset="utf-8">)"
      R"(    <meta name="viewport" content="width=device-width, initial-scale=1">)"
      R"(    <meta name="description" content="">)"
      R"(    <meta name="author" content="">)"
      R"(    <title>)",
      title.ToString(),
      R"(</title>)"
      R"(    <link href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css" rel="stylesheet">)"
      R"(    <link href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap-theme.min.css" rel="stylesheet">)"
      R"(    <style>pre { white-space: pre-wrap; word-wrap: break-word; }</style>)"
      R"(  </head>)"
      R"(  <body style="padding-top:70px;padding-bottom:30px">)"_a);

  const vector<pair<string, string>> links = {
      {"Info", "/_/"},
      {"Stats", "/_/stats"},
      {"DB Status", "/_/db"},
      {"Session", "/_/session"},
  };

  handler->Print(
      R"(    <nav class="navbar navbar-inverse navbar-fixed-top">)"
      R"(      <div class="container">)"
      R"(        <div class="navbar-header">)"
      R"(          <button type="button" class="navbar-toggle collapsed" data-toggle="collapse" data-target="#navbar" aria-expanded="false" aria-controls="navbar">)"
      R"(            <span class="sr-only">Toggle navigation</span>)"
      R"(            <span class="icon-bar"></span>)"
      R"(            <span class="icon-bar"></span>)"
      R"(            <span class="icon-bar"></span>)"
      R"(          </button>)"
      R"(          <a class="navbar-brand" href="/_/">Nineserver</a>)"
      R"(        </div>)"
      R"(        <div id="navbar" class="navbar-collapse collapse">)"
      R"(          <ul class="nav navbar-nav">)"_a);

  StringPiece script_name = handler->ENV("SCRIPT_NAME");
  for (const auto& link : links) {
    handler->Print(
        link.second == script_name ? R"(<li class="active">)"_a : "<li>"_a);
    handler->Print("<a href=\""_a, string(link.second), "\">"_a,
                   string(link.first), "</a></li>"_a);
  }

  handler->Print(
      R"(                </ul>)"
      R"(              </li>)"
      R"(            </ul>)"
      R"(          </div><!--/.nav-collapse -->)"
      R"(        </div>)"
      R"(      </nav>)"
      R"(<div class="container theme-showcase" role="main">)"_a);
}

SystemHandlerTemplate::~SystemHandlerTemplate() {
  handler_->Print(
      R"(</div>)"
      R"(    <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js"></script>)"
      R"(    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js"></script>)"
      R"(    <script src="https://cdn.rawgit.com/google/code-prettify/master/loader/run_prettify.js"></script>)"
      R"(  </body>)"
      R"(</html>)"
  );
}

void SystemHandlerTemplate::Header(StringPiece title) {
  handler_->Print(
      R"(<div class="page-header"><h2>)"_a, title.ToString(), "</h2></div>"_a);
}
