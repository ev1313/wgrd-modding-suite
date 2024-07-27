#include "ndftransactions.hpp"

#include <iostream>

void wgrd_files::NdfBinFile::start_parsing(fs::path path) {
  spdlog::warn("start_parsing {}", path.string());
  ndf_parsed = false;
  assert(ndf_parsing == false);
  m_ndf_parsed_promise = std::nullopt;
  m_ndf_parsed_future = std::nullopt;
  ndf.clear();

  m_ndf_parsed_promise = std::promise<bool>();
  m_ndf_parsed_future = m_ndf_parsed_promise->get_future();
    
  py::gil_scoped_acquire acquire;
  py::object decompress_ndfbin = py::module_::import("wgrd_cons_parsers.decompress_ndfbin").attr("decompress_ndfbin");
  py::object open = py::module_::import("io").attr("open");

  fs::create_directories(path.parent_path());

  py::object f = open(path.string(), "rb");
  py::object data = f.attr("read")();
  f.attr("close")();
  
  py::object decompressed_data = decompress_ndfbin(data);

  fs::path decompressed_path = path;
  decompressed_path.replace_extension("decompressed.ndfbin");

  py::object of = open(decompressed_path.string(), "wb");
  of.attr("write")(decompressed_data);
  of.attr("close")();
  py::gil_scoped_release release;

  fs::path outpath_xml = decompressed_path;
  outpath_xml.replace_extension(".xml");

  ndf.load_from_ndfbin(decompressed_path);

  this->m_ndf_parsed_promise->set_value(true);

  ndf_parsing = true;
}


