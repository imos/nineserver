#pragma once
#include "nineserver/handler/base_handler.h"

#define SYSTEM_HANDLER_PREFIX "/_"

class SystemHandlerTemplate {
 public:
  SystemHandlerTemplate(StringPiece title, BaseHandler* handler);
  ~SystemHandlerTemplate();

  void Header(StringPiece title);

 private:
  BaseHandler* handler_;
};
