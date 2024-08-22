#include "ess.hpp"
#include "helpers.hpp"

#include <imgui.h>

void wgrd_files::Ess::render_window() {
  ImGui::Text("Ess: %s", meta.vfs_path.c_str());
  ImGui::Text("Loop Start: %d", loop_start);
  ImGui::Text("Loop End: %d", loop_end);
}

bool wgrd_files::Ess::is_file(const FileMeta &meta) {
  auto stream_opt = open_file(meta.fs_path);
  if (!stream_opt) {
    return false;
  }
  auto &stream = stream_opt.value();
  stream.seekg(meta.offset);

  char magic[4];
  stream.read(magic, sizeof(magic));

  if (magic[0] == 0x01 && magic[1] == 0x00 && magic[2] == 0x02 &&
      magic[3] == 0x02) {
    return true;
  }
  return false;
}

bool wgrd_files::Ess::load_bin() {
  spdlog::info("Parsing Ess: {}", meta.vfs_path);

  try {
    py::gil_scoped_acquire acquire;
    // we decode the ess file to xml so we get access to loop start / end
    py::object ess = py::module::import("wgrd_cons_parsers.ess").attr("Ess");
    std::vector<char> vec_data = get_data();
    py::bytes data(vec_data.data(), vec_data.size());
    py::object parsed = ess.attr("parse")(data);
    spdlog::debug("parsed ess successfully {} {}", py::len(parsed),
                  py::str(parsed).cast<std::string>());

    loop_start = parsed["loopStart"].cast<uint32_t>();
    loop_end = parsed["loopEnd"].cast<uint32_t>();

    // and now we decode it to wav file, so we can play it
    py::object decode_ess =
        py::module::import("wgrd_cons_tools.decode_ess").attr("decode_ess");
    fs::path wav_path = xml_path.replace_extension(".ess.wav");
    fs::path labels_path = xml_path.replace_extension(".ess.labels");
    fs::create_directories(xml_path.parent_path());
    decode_ess(data, wav_path.string(), labels_path.string());

  } catch (const py::error_already_set &e) {
    spdlog::error("Error parsing Ess: {}", e.what());
    return false;
  }

  return true;
}
