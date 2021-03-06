#pragma once

#if defined(SQLDSML_HPP_LOG_FILENAME)

#include <ctime>
#include <fstream>
#include <sstream>
#include <iostream>

namespace sqldsml {
  class logging {
  public:
    static logging& get_instance() {
      static logging instance;
      return instance;
    }

    void log(const std::string& s) {
      log_to_stream(log_, s);
    }
  
  private:
    logging() {
      log_.open(SQLDSML_HPP_LOG_FILENAME);
      log("Sqlite header-only library logging started");
    }
    logging(logging const&) = delete;
    void operator=(logging const&) = delete;

    void log_to_stream(std::ofstream& out, const std::string& s) {
      std::time_t now = std::time(0);
      std::tm stm;
#if defined(_MSC_VER) || defined(WIN32) || defined(_WIN32)
      localtime_s(&stm, &now);
#else
      localtime_r(&now, &stm);
#endif
      char buffer[256];
      std::strftime(buffer, 256, "%Y.%m.%d %H:%M:%S", &stm);
      out << buffer << " ";
      out << s;
      out << "\n";
      log_.flush();
    }

    std::ofstream log_;
  };
}

#define SQLDSML_HPP_LOG(X) ::sqldsml::logging::get_instance().log(X);
#else
#define SQLDSML_HPP_LOG(X)
#endif
