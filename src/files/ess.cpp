#include "ess.hpp"

#include <imgui.h>

wgrd_files::Ess::Ess(FileMeta meta, fs::path out_path) : File(meta, out_path) {
  load_bin();
}

bool wgrd_files::Ess::render() {
  ImGui::Text("Ess: %s", vfs_path.c_str());
  ImGui::Text("Loop Start: %d", loop_start);
  ImGui::Text("Loop End: %d", loop_end);
  return true;
}

bool wgrd_files::Ess::is_file(std::string vfs_path, std::ifstream &f, size_t offset) {
  f.clear();
  f.seekg(offset);

  char magic[4];
  f.read(magic, sizeof(magic));

  f.clear();
  f.seekg(offset);

  if(magic[0] == 0x01 && magic[1] == 0x00 && magic[2] == 0x02 && magic[3] == 0x02) {
    return true;
  }
  return false;
}

bool wgrd_files::Ess::load_bin() {
  spdlog::info("Parsing Ess: {}", vfs_path);
  py::gil_scoped_acquire acquire;

  try {
    // we decode the ess file to xml so we get access to loop start / end
    py::object ess = py::module::import("wgrd_cons_parsers.ess").attr("Ess");
    std::vector<char> vec_data = get_data();
    py::bytes data(vec_data.data(), vec_data.size());
    py::object parsed = ess.attr("parse")(data);
    spdlog::debug("parsed ess successfully {} {}", py::len(parsed), py::str(parsed).cast<std::string>());

    loop_start = parsed["loopStart"].cast<uint32_t>();
    loop_end = parsed["loopEnd"].cast<uint32_t>();

    // and now we decode it to wav file, so we can play it
    py::object decode_ess = py::module::import("wgrd_cons_tools.decode_ess").attr("decode_ess");
    std::string wav_path = out_path / "xml" / fs::path(vfs_path).replace_extension(".ess.wav");
    std::string labels_path = out_path / "xml" / fs::path(vfs_path).replace_extension(".ess.labels");
    fs::create_directories(out_path / "xml" / vfs_path);
    decode_ess(data, wav_path, labels_path);

  } catch(const py::error_already_set &e) {
    spdlog::error("Error parsing Ess: {}", e.what());
    return false;
  }

  py::gil_scoped_release release;
  is_parsed = true;
  return true;
}
