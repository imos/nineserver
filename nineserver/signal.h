#pragma once
#include <functional>

void AddExitHandler(const std::function<void()>& handler);
void RunExitHandlers();
