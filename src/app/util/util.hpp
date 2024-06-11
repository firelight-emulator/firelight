#pragma once

#include <sstream>
#include <string>
#include <thread>

static std::string thread_id_string() {
  std::stringstream ss;
  ss << std::this_thread::get_id();
  return ss.str();
}