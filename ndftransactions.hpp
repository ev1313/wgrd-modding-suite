#pragma once

#include <future>
#include <optional>
#include <pugixml.hpp>

#include <filesystem>
namespace fs = std::filesystem;

#include <pybind11/embed.h>
namespace py = pybind11;

#include <fstream>

#include "ndf.hpp"

namespace wgrd_files {

/*
 * List of possible transactions
 *
 * - add object with class id (properties are all empty)
 * - remove object => ultra aids, will rewrite EVERY id
 * - change object class id (-> == remove object, add object with class id)
 * - edit object property
 * - change export name
 * - set as top object
 *
 *
 * */


class NdfBinFile {
private:
  bool ndf_parsed = false;
  bool ndf_parsing = false;
  std::optional<std::promise<bool>> m_ndf_parsed_promise;
  std::optional<std::future<bool>> m_ndf_parsed_future;
  NDF ndf;

public:
  void start_parsing(fs::path path);
  bool load_from_file(fs::path path);

  bool is_parsing() {
    return ndf_parsing;
  }
  bool is_parsed() {
    return ndf_parsed;
  }
  void check_parsing() {
    if(!m_ndf_parsed_promise.has_value()) {
      throw std::runtime_error("m_ndf_parsed_promise not set");
    }
    if(!m_ndf_parsed_future.has_value()) {
      throw std::runtime_error("m_ndf_parsed_future not set");
    }
    if(m_ndf_parsed_future.value().wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
      ndf_parsed = m_ndf_parsed_future.value().get();
      ndf_parsing = false;
    }
  }
  size_t get_object_count() {
    return ndf.objects.size();
  }

  NDFObject& get_object(size_t index) {
    return ndf.objects.at(index);
  }
};

}
