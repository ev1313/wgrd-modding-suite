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

std::optional<FileMeta> FileTree::file_list(py::dict files, const std::string& vfs_path) {
  std::optional<FileMeta> ret = std::nullopt;
  static int selected_file = -1;
  for(auto [path, value] : files) {
    std::string full_vfs_path = vfs_path + "/" + path.cast<std::string>();
    assert(py::isinstance<py::tuple>(value) || py::isinstance<py::dict>(value));
    if (py::isinstance<py::dict>(value)) {
      auto sub_ret = file_list(value.cast<py::dict>(), full_vfs_path);
      if(!ret.has_value()) {
        ret = sub_ret;
      }
    } else {
      unsigned int idx = value.cast<py::tuple>()[4].cast<unsigned int>();
      if(ImGui::Selectable(full_vfs_path.c_str(), selected_file == idx)) {
        selected_file = idx;
        //ImGui::Text("File %d: %s offset 0x%zX size 0x%zX", idx, file.cast<std::string>().c_str(), offset, size);
      }
      if(idx == selected_file) {
        py::str file = value.cast<py::tuple>()[0].cast<py::str>();
        size_t offset = value.cast<py::tuple>()[1].cast<size_t>();
        size_t size = value.cast<py::tuple>()[2].cast<size_t>();
        ret = FileMeta(full_vfs_path, file.cast<std::string>(), offset, size, idx);

        ImGui::SetItemDefaultFocus();
      }
    }
  }

  return ret;
}

std::optional<FileMeta> FileTree::file_tree(py::dict files, const std::string& vfs_path, bool open_all) {
  std::optional<FileMeta> ret = std::nullopt;
  static int selected_file = -1;
  for(auto [path, value] : files) {
    std::string full_vfs_path = vfs_path + "/" + path.cast<std::string>();
    assert(py::isinstance<py::tuple>(value) || py::isinstance<py::dict>(value));
    // dict -> has children
    if (py::isinstance<py::dict>(value)) {
      if(open_all) {
        ImGui::SetNextItemOpen(true);
      }
      if(ImGui::TreeNodeEx(path.cast<std::string>().c_str(), ImGuiTreeNodeFlags_NavLeftJumpsBackHere)) {
        bool child_open_all = false;
        if(!ImGui::IsItemToggledOpen() && ImGui::IsItemHovered() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) {
          child_open_all = true;
        }
        auto sub_ret = file_tree(value.cast<py::dict>(), full_vfs_path, open_all || child_open_all);
        if(!ret.has_value()) {
          ret = sub_ret;
        }

        ImGui::TreePop();
      }
      // tuple -> no it is a file, is (file, offset, size, checksum, idx)
    } else {
      unsigned int idx = value.cast<py::tuple>()[4].cast<unsigned int>();
      if(ImGui::Selectable(path.cast<std::string>().c_str(), selected_file == idx)) {
        selected_file = idx;
        //ImGui::Text("File %d: %s offset 0x%zX size 0x%zX", idx, file.cast<std::string>().c_str(), offset, size);
      }
      if(idx == selected_file) {
        py::str file = value.cast<py::tuple>()[0].cast<py::str>();
        size_t offset = value.cast<py::tuple>()[1].cast<size_t>();
        size_t size = value.cast<py::tuple>()[2].cast<size_t>();
        ret = FileMeta(full_vfs_path, file.cast<std::string>(), offset, size, idx);
        
        ImGui::SetItemDefaultFocus();
      }
    }
  }
  return ret;
}

std::optional<FileMeta> FileTree::render(const std::string& name) {
  py::gil_scoped_acquire acquire;
  py::object create_vfs_tree = py::module_::import("wgrd_cons_tools.create_vfs").attr("create_vfs_tree");
  py::object search_vfs_tree = py::module_::import("wgrd_cons_tools.create_vfs").attr("search_vfs_tree");

  static bool rebuild_tree = true;

  ImGui::BeginChild(name.c_str(), ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Border | ImGuiChildFlags_FrameStyle);

  static std::string search;
  ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.9f);
  rebuild_tree |= ImGui::InputText("##file_tree_search", &search);

  static bool tree_view = true;
  ImGui::Checkbox("Tree View", &tree_view);

  py::dict files = vfs_files;
  if(rebuild_tree) {
    if(!search.empty()) {
      files = search_vfs_tree(vfs_files, search);
    }
    vfs_tree = create_vfs_tree(files);
    rebuild_tree = false;
  }
  std::optional<FileMeta> ret = std::nullopt;
  if(tree_view) {
    ret = file_tree(vfs_tree);
  } else {
    ImGui::BeginListBox("##Filelist", ImVec2(ImGui::GetContentRegionAvail().x * 0.9f, ImGui::GetContentRegionAvail().y));
    ret = file_list(vfs_tree);
    ImGui::EndListBox();
  }
  ImGui::EndChild();
  py::gil_scoped_release release;
  return ret;
}
