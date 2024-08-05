#include "ndftransactions.hpp"

void wgrd_files::NdfBinFile::start_parsing(fs::path vfs_path, fs::path file_path) {
  std::ifstream file(file_path, std::ios::binary | std::ios::in);
  std::vector<char> data;
  data.resize(fs::file_size(file_path));
  file.read(data.data(), data.size());
  start_parsing(vfs_path, data);
}

void wgrd_files::NdfBinFile::start_parsing(fs::path vfs_path, std::vector<char> vec_data) {
  spdlog::info("start_parsing {}", vfs_path.string());
  ndf_parsed = false;
  assert(ndf_parsing == false);
  m_ndf_parsed_promise = std::nullopt;
  m_ndf_parsed_future = std::nullopt;
  ndf.clear();

  //m_ndf_parsed_promise = std::promise<bool>();
  //m_ndf_parsed_future = m_ndf_parsed_promise->get_future();
    
  py::gil_scoped_acquire acquire;
  try {
    py::object decompress_ndfbin = py::module_::import("wgrd_cons_parsers.decompress_ndfbin").attr("decompress_ndfbin");

    py::bytes data(vec_data.data(), vec_data.size());

    py::bytes decompressed_data = decompress_ndfbin(data);

    std::stringstream decompressed_stream(decompressed_data);
    ndf.load_from_ndfbin_stream(decompressed_stream);
  } catch (py::error_already_set &e) {
    spdlog::error("Error parsing NDF: {}", e.what());
  }
  py::gil_scoped_release release;

  //this->m_ndf_parsed_promise->set_value(true);

  // change this to ndf_parsing when doing async things
  ndf_parsed = true;
  ndf_parsing = false;
}

