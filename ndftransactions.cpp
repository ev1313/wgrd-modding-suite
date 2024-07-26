#include "ndftransactions.hpp"

#include "ndfbin.hpp"

void wgrd_files::NdfBinFile::start_parsing(fs::path path) {
  m_ndf_parsed_promise = std::nullopt;
  m_ndf_parsed_future = std::nullopt;
  ndf_xml.reset();

  m_ndf_parsed_promise = std::promise<bool>();
  m_ndf_parsed_future = m_ndf_parsed_promise->get_future();

  std::thread([&]() {
    py::gil_scoped_acquire acquire;
    py::object decompress_ndfbin = py::module_::import("wgrd_cons_parsers.decompress_ndfbin").attr("decompress_ndfbin ");
    py::object open = py::module_::import("io").attr("open");

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

    {
      auto NdfBin = get_ndfbin();
      std::ifstream input;
      spdlog::debug("Opening NdfBin {}", decompressed_path.string());
      input.open(decompressed_path, std::ios::in | std::ios::binary);
      NdfBin->parse(input);

      pugi::xml_document doc;
      auto root = doc.append_child("root");
      NdfBin->build_xml(root, "NdfBin");
      doc.save_file(outpath_xml.c_str());
    }

    pugi::xml_parse_result result = ndf_xml.load_file(outpath_xml.c_str());

    this->m_ndf_parsed_promise->set_value(true);
  }).detach();
  ndf_parsing = true;
}


