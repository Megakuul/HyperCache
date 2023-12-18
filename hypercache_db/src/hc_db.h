#pragma once

#include <vector>
#include <string>


#ifdef _WIN32
  #define HC_DB_EXPORT __declspec(dllexport)
#else
  #define HC_DB_EXPORT
#endif

HC_DB_EXPORT void hc_db();
HC_DB_EXPORT void hc_db_print_vector(const std::vector<std::string> &strings);
