#pragma once

#include <string>
#include <toml.hpp>

#include <spdlog/spdlog.h>

#include <filesystem>
namespace fs = std::filesystem;

enum class FileType {
  DIC,
  EDAT,
  ESS,
  NDFBIN,
  PPK,
  SCENARIO,
  SFORMAT,
  TGV,
  UNKNOWN
};

struct WorkspaceConfig {
  // name of the workspace
  std::string name;
  // path to the input files
  fs::path fs_path;
  // output folder for the generated dat files
  fs::path dat_path;
  // output folder for loaded and generated bin files
  // (mostly extracted from dat files or generated for dat files)
  fs::path bin_path;
  // output folder for loaded and generated xml files
  fs::path xml_path;
  // path to the databases, shared between all files
  fs::path db_path;
  // temp folder
  fs::path tmp_path;
  toml::table to_toml() {
    toml::table table;
    table["name"] = name;
    table["fs_path"] = fs_path.string();
    table["dat_path"] = dat_path.string();
    table["bin_path"] = bin_path.string();
    table["xml_path"] = xml_path.string();
    table["db_path"] = db_path.string();
    table["tmp_path"] = tmp_path.string();
    return table;
  }
  bool from_toml(toml::table table) noexcept {
    try {
      name = table["name"].as_string();
      fs_path = table["fs_path"].as_string();
      dat_path = table["dat_path"].as_string();
      bin_path = table["bin_path"].as_string();
      xml_path = table["xml_path"].as_string();
      db_path = table["db_path"].as_string();
      tmp_path = table["tmp_path"].as_string();
    } catch (const toml::type_error &e) {
      spdlog::error("error while parsing workspace config: {}", e.what());
      return false;
    }
    return true;
  }
};
