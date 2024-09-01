#include "ndftransactions.hpp"
#include "helpers.hpp"

void wgrd_files::NdfBinFile::start_parsing(fs::path vfs_path,
                                           fs::path file_path) {
  std::ifstream file(file_path, std::ios::binary | std::ios::in);
  std::vector<char> data;
  data.resize(fs::file_size(file_path));
  file.read(data.data(), data.size());
  start_parsing(vfs_path, data);
}

void wgrd_files::NdfBinFile::start_parsing(fs::path vfs_path,
                                           std::vector<char> vec_data) {
  spdlog::info("loading ndfbin from bin {}", vfs_path.string());
  ndf.clear();

  {
    py::gil_scoped_acquire acquire;
    try {
      py::object decompress_ndfbin =
          py::module_::import("wgrd_cons_parsers.decompress_ndfbin")
              .attr("decompress_ndfbin");

      py::bytes data(vec_data.data(), vec_data.size());

      py::bytes decompressed_data = decompress_ndfbin(data);

      std::stringstream decompressed_stream(decompressed_data);
      ndf.load_from_ndfbin_stream(decompressed_stream);
    } catch (py::error_already_set &e) {
      spdlog::error("Error parsing NDF: {}", e.what());
    }
  }
}

void wgrd_files::NdfBinFile::load_from_xml_file(fs::path path, NDF_DB *db,
                                                int ndf_id) {
  spdlog::info("Loading ndfbin from xml {}", path.string());
  ndf.clear();
  ndf.load_from_ndf_xml(path, db, ndf_id);
}
