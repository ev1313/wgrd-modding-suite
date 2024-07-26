#include "file_tree.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <iostream>

#include "spdlog/spdlog.h"

bool FileTree::init_from_wgrd_path(fs::path wgrd_path) {
  try {
    py::object get_dat_paths = py::module_::import("wgrd_cons_tools.create_vfs").attr("get_dat_paths");
    py::object create_vfs = py::module_::import("wgrd_cons_tools.create_vfs").attr("create_vfs");
    std::cout << wgrd_path / "Maps/" << std::endl;
    py::object dat_paths_maps = get_dat_paths((wgrd_path / "Maps/").string());
    py::object dat_paths_data = get_dat_paths((wgrd_path / "Data/WarGame/").string());
    vfs_files = create_vfs(dat_paths_data);
  } catch (py::error_already_set& e) {
    spdlog::error(e.what());
    return false;
  }
  return true;
}

bool FileTree::init_from_path(fs::path path) {
  try {
    py::object create_vfs = py::module_::import("wgrd_cons_tools.create_vfs").attr("create_vfs");
    vfs_files = create_vfs(py::list({py::str(path)}));
  } catch (py::error_already_set& e) {
    spdlog::error(e.what());
    return false;
  }
  return true;
}

std::vector<FileMeta> FileTree::file_tree(py::dict files, const std::string& vfs_path, bool open_all) {
  std::vector<FileMeta> ret;
  for(auto [path, value] : files) {
    if(open_all) {
      ImGui::SetNextItemOpen(true);
    }
    bool open = ImGui::TreeNodeEx(path.cast<std::string>().c_str(), ImGuiTreeNodeFlags_NavLeftJumpsBackHere);
    if(open) {
      bool child_open_all = false;
      if(!ImGui::IsItemToggledOpen() && ImGui::IsItemHovered() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) {
        child_open_all = true;
      }
      if(py::isinstance<py::tuple>(value)) {
        py::str file = value.cast<py::tuple>()[0].cast<py::str>();
        size_t offset = value.cast<py::tuple>()[1].cast<size_t>();
        size_t size = value.cast<py::tuple>()[2].cast<size_t>();
        ImGui::Text("File: %s offset 0x%zX size 0x%zX", file.cast<std::string>().c_str(), offset, size);
        bool file_opened = ImGui::Button("Open");
        if(file_opened) {
          ret.emplace_back(vfs_path + "/" + path.cast<std::string>(), file.cast<std::string>(), offset, size);
        }
      } else
      if (py::isinstance<py::dict>(value)) {
        auto sub_ret = file_tree(value.cast<py::dict>(), vfs_path + "/" + path.cast<std::string>(), open_all || child_open_all);
        ret.insert(ret.end(), std::make_move_iterator(sub_ret.begin()), std::make_move_iterator(sub_ret.end()));
      } else {
        assert(false);
      }

      ImGui::TreePop();
    }
  }
  return ret;
}

std::vector<FileMeta> FileTree::render(const std::string& name) {
  py::object create_vfs_tree = py::module_::import("wgrd_cons_tools.create_vfs").attr("create_vfs_tree");
  py::object search_vfs_tree = py::module_::import("wgrd_cons_tools.create_vfs").attr("search_vfs_tree");

  static bool rebuild_tree = true;

  ImGui::Begin(name.c_str());
  static std::string search;
  rebuild_tree |= ImGui::InputText("##file_tree_search", &search);

  py::dict files = vfs_files;
  if(rebuild_tree) {
    if(!search.empty()) {
      files = search_vfs_tree(vfs_files, search);
    }
    vfs_tree = create_vfs_tree(files);
    rebuild_tree = false;
  }

  std::vector<FileMeta> ret = file_tree(vfs_tree);
  ImGui::End();
  return ret;
}
