#include "dic.hpp"
#include "helpers.hpp"

#include "helpers.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>

using namespace pybind11::literals;

wgrd_files::Dic::Dic(FileMeta meta, fs::path out_path) : File(meta, out_path) {}

bool wgrd_files::Dic::load_stream() {
  spdlog::info("Parsing Dic: {}", vfs_path);

  try {
    py::gil_scoped_acquire acquire;
    py::object dic = py::module::import("wgrd_cons_parsers.dic").attr("Dic");
    std::vector<char> vec_data = get_data();
    py::bytes data(vec_data.data(), vec_data.size());
    py::object parsed = dic.attr("parse")(data);
    spdlog::debug("parsed dic successfully {} {}", py::len(parsed),
                  py::str(parsed).cast<std::string>());
    for (auto &entry : parsed["entries"]) {
      spdlog::debug("Entry: {}", py::str(entry).cast<std::string>());
      std::string hash =
          py::str(entry["hash"].attr("hex")()).cast<std::string>();
      std::string str = py::str(entry["string"]).cast<std::string>();
      spdlog::debug("Dic Hash: {}, String: {}", hash, str);
      dic_data[hash] = str;
    }
  } catch (const py::error_already_set &e) {
    spdlog::error("Error parsing Dic: {}", e.what());
    return false;
  }

  return true;
}

bool wgrd_files::Dic::load_xml(fs::path path) {
  spdlog::info("Loading Dic XML: {} from {}", vfs_path, path.string());

  try {
    py::gil_scoped_acquire acquire;
    py::module ET = py::module::import("xml.etree.ElementTree");
    py::object dic = py::module::import("wgrd_cons_parsers.dic").attr("Dic");
    py::object xml = ET.attr("parse")(path.string());
    py::dict py_dic_data = dic.attr("fromET")(xml.attr("getroot")());
    for (auto &entry : py_dic_data["entries"]) {
      std::string hash =
          py::str(entry["hash"].attr("hex")()).cast<std::string>();
      std::string str = py::str(entry["string"]).cast<std::string>();
      dic_data[hash] = str;
    }
  } catch (const py::error_already_set &e) {
    spdlog::error("Error loading Dic XML: {}", e.what());
    return false;
  }

  return true;
}

bool wgrd_files::Dic::save_xml(fs::path path) {
  spdlog::info("Saving Dic XML: {} to {}", vfs_path, path.string());

  try {
    py::gil_scoped_acquire acquire;
    py::dict py_dic_data;
    py_dic_data["entries"] = py::list();
    for (auto &[hash, str] : dic_data) {
      py::bytes hash_bytes;
      hash_bytes = hash_bytes.attr("fromhex")(py::str(hash));
      py::dict entry;
      entry["hash"] = hash_bytes;
      entry["string"] = py::str(str);
      py_dic_data["entries"].attr("append")(entry);
    }

    py::module ET = py::module::import("xml.etree.ElementTree");
    py::object dic = py::module::import("wgrd_cons_parsers.dic").attr("Dic");
    py::object xml =
        dic.attr("toET")(py_dic_data, "name"_a = "Dic", "is_root"_a = false);
    ET.attr("indent")(xml, "space"_a = "  ", "level"_a = 0);
    py::str xml_string = ET.attr("tostring")(xml).attr("decode")("utf-8");
    fs::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::out | std::ios::trunc);
    file << xml_string.cast<std::string>();
  } catch (const py::error_already_set &e) {
    spdlog::error("Error saving Dic XML: {}", e.what());
    return false;
  }

  return true;
}

bool wgrd_files::Dic::save_bin(fs::path path) {
  spdlog::info("Saving Dic: {}", vfs_path);

  try {
    py::gil_scoped_acquire acquire;
    py::dict py_dic_data;
    py_dic_data["entries"] = py::list();
    for (auto &[hash, str] : dic_data) {
      py::bytes hash_bytes;
      hash_bytes = hash_bytes.attr("fromhex")(py::str(hash));
      py::dict entry;
      entry["hash"] = hash_bytes;
      entry["string"] = py::str(str);
      py_dic_data["entries"].attr("append")(entry);
    }

    py::object dic = py::module::import("wgrd_cons_parsers.dic").attr("Dic");
    py::tuple preprocessed_tuple = dic.attr("preprocess")(py_dic_data);
    py::object preprocessed = preprocessed_tuple[0];
    fs::create_directories(path.parent_path());
    dic.attr("build_file")(preprocessed, path.string());
  } catch (const py::error_already_set &e) {
    spdlog::error("Error saving Dic: {}", e.what());
    return false;
  }

  m_is_changed = false;
  return true;
}

void wgrd_files::Dic::render_window() {
  if (ImGui::BeginTable(gettext("Dictionary entries"), 2,
                        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
    ImGui::TableSetupColumn(gettext("Hash"), ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn(gettext("String"),
                            ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();
    ImGui::TableNextColumn();
    for (auto &[hash, str] : dic_data) {
      if (hash == "0000000000000080") {
        continue;
      }
      ImGui::PushID(hash.c_str());
      std::string hash_str = hash;
      ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
      if (ImGui::InputText("##DicHashInput", &hash_str,
                           ImGuiInputTextFlags_EnterReturnsTrue)) {
        auto trans = std::make_unique<DicTransaction_ChangeHash>();
        trans->previous_hash = hash;
        trans->new_hash = hash_str;
        transactions.push_back(std::move(trans));
        transactions.back()->apply(dic_data);
        m_is_changed = true;
      }
      ImGui::TableNextColumn();
      std::string value_str = str;
      ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
      if (ImGui::InputText("##DicValueInput", &value_str,
                           ImGuiInputTextFlags_EnterReturnsTrue)) {
        auto trans = std::make_unique<DicTransaction_ChangeValue>();
        trans->hash = hash;
        trans->new_value = value_str;
        transactions.push_back(std::move(trans));
        transactions.back()->apply(dic_data);
        m_is_changed = true;
      }
      ImGui::TableNextColumn();
      ImGui::PopID();
    }
    ImGui::EndTable();
  }
}

bool wgrd_files::Dic::is_file(std::string vfs_path, std::ifstream &f,
                              size_t offset) {
  f.clear();
  f.seekg(offset);

  char magic[4];
  f.read(magic, sizeof(magic));

  f.clear();
  f.seekg(offset);

  if (!strncmp(magic, "TRAD", 4)) {
    return true;
  }
  return false;
}
