#include "file_tree.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <iostream>
#include <memory>

#include "helpers.hpp"
#include "spdlog/spdlog.h"

bool FileTree::init_from_wgrd_path(fs::path wgrd_path) {
  spdlog::info("parsing filetree from wgrd_path {}", wgrd_path.string());
  {
    py::gil_scoped_acquire acquire;
    try {
      py::object get_dat_paths = py::module_::import("wgrd_cons_tools.create_vfs").attr("get_dat_paths");
      py::object create_vfs = py::module_::import("wgrd_cons_tools.create_vfs").attr("create_vfs");
      py::object create_vfs_tree = py::module_::import("wgrd_cons_tools.create_vfs").attr("create_vfs_tree");
      std::cout << wgrd_path / "Maps/" << std::endl;
      py::object dat_paths_maps = get_dat_paths((wgrd_path / "Maps/").string());
      py::object dat_paths_data = get_dat_paths((wgrd_path / "Data/WarGame/").string());
      py::dict files = create_vfs(dat_paths_data);
      // FIXME: do we need to create the tree here?
      // we could just do it in C++
      py::dict tree = create_vfs_tree(files);
      fill_filetree(tree, "$");
    } catch (py::error_already_set& e) {
      spdlog::error(e.what());
      return false;
    }
  }
  spdlog::info("parsed filetree from wgrd_path {}", wgrd_path.string());
  return true;
}

bool FileTree::init_from_dat_path(fs::path path) {
  spdlog::info("parsing filetree from dat_path {}", path.string());
  {
    py::gil_scoped_acquire acquire;
    try {
      py::object get_dat_paths = py::module_::import("wgrd_cons_tools.create_vfs").attr("get_dat_paths");
      py::object create_vfs = py::module_::import("wgrd_cons_tools.create_vfs").attr("create_vfs");
      py::object create_vfs_tree = py::module_::import("wgrd_cons_tools.create_vfs").attr("create_vfs_tree");
      py::object dat_paths = get_dat_paths((path).string());
      py::dict files = create_vfs(dat_paths);
      py::dict tree = create_vfs_tree(files);
      fill_filetree(tree, "$");
    } catch (py::error_already_set& e) {
      spdlog::error(e.what());
      return false;
    }
  }
  spdlog::info("parsed filetree from dat_path {}", path.string());
  return true;
}

bool FileTree::init_from_path(fs::path path) {
  spdlog::info("parsing filetree from path {}", path.string());
  {
    py::gil_scoped_acquire acquire;
    try {
      py::object create_vfs = py::module_::import("wgrd_cons_tools.create_vfs").attr("create_vfs");
      py::object create_vfs_tree = py::module_::import("wgrd_cons_tools.create_vfs").attr("create_vfs_tree");
      py::list lst;
      lst.attr("append")(py::str(path.string()));
      py::dict files = create_vfs(lst);
      py::dict tree = create_vfs_tree(files);
      fill_filetree(tree, "$");
    } catch (py::error_already_set& e) {
      spdlog::error(e.what());
      return false;
    }
  }
  spdlog::info("parsed filetree from path {}", path.string());
  return true;
}

bool FileTree::init_from_stream(std::ifstream& stream) {
  throw std::runtime_error("Not implemented");
  return false;
}

void FileTree::fill_filetree(py::dict files, const std::string& vfs_path) {
  for(auto [path, value] : files) {
    //spdlog::debug("parsing filetree item {}", path.cast<std::string>());
    std::string full_vfs_path = vfs_path + "/" + path.cast<std::string>();
    assert(py::isinstance<py::tuple>(value) || py::isinstance<py::dict>(value));
    if(!vfs_string_table.contains(path.cast<std::string>())) {
      vfs_string_table[path.cast<std::string>()] = vfs_string_table.size();
    }
    if (py::isinstance<py::dict>(value)) {
      fill_filetree(value.cast<py::dict>(), full_vfs_path);
    } else {
      //spdlog::debug("inserting {}", full_vfs_path);
      py::str file = value.cast<py::tuple>()[0].cast<py::str>();
      size_t offset = value.cast<py::tuple>()[1].cast<size_t>();
      size_t size = value.cast<py::tuple>()[2].cast<size_t>();
      unsigned int idx = value.cast<py::tuple>()[4].cast<unsigned int>();
      auto meta = FileMeta(full_vfs_path, file.cast<std::string>(), offset, size, idx);
      vfs_files[full_vfs_path] = meta;
      std::vector<uint32_t> vec;
      for(auto str_it : std::views::split(remove_dollar(full_vfs_path), '/')) {
        std::string str = std::string(str_it.begin(), str_it.end());
        vec.push_back(vfs_string_table[str]);
      }
      vfs_indexed_files[vec] = full_vfs_path;
    }
  }
  if(vfs_path == "$") {
    filter_filetree();
  }
}

void FileTree::filter_filetree() {
  if(m_search_lower.empty()) {
    vfs_filtered_files = vfs_indexed_files;
    return;
  }
  vfs_filtered_files.clear();
  for(auto& [vfs_indexed_path, vfs_path] : vfs_indexed_files) {
    if(str_tolower(vfs_path).contains(m_search_lower)) {
      vfs_filtered_files[vfs_indexed_path] = vfs_path;
    }
  }
}

std::optional<FileMeta> FileTree::render_file_list() {
  std::optional<FileMeta> ret = std::nullopt;

  ImGuiListClipper clipper;
  clipper.Begin(vfs_filtered_files.size());
  auto vfs_it = vfs_filtered_files.begin();
  while(clipper.Step()) {
    for(int it = 0; it < clipper.DisplayEnd && vfs_it != vfs_filtered_files.end(); ) {
      // we only need to increment the vfs iterator, if it is currently shown
      auto [_, vfs_path] = *vfs_it;
      // we need to first get to the display start
      if(it < clipper.DisplayStart) {
        it++;
        continue;
      }
      // now we are inside the range we want to display
      if(ImGui::Selectable(vfs_path.c_str(), selected_vfs_path == vfs_path)) {
        selected_vfs_path = vfs_path;
        ret = vfs_files[vfs_path];
      }

      if(vfs_path == selected_vfs_path) {
        ImGui::SetItemDefaultFocus();
      }
      // after item is rendered, inc both iterators
      vfs_it++;
      it++;
    }
  }
  clipper.End();

  return ret;
}

std::optional<FileMeta> FileTree::render_file_tree() {
  // FIXME: https://github.com/ocornut/imgui/issues/3823
  std::optional<FileMeta> ret = std::nullopt;

  // for keyboard navigation,
  // index of the level from which all nodes shall be open
  uint32_t child_open_all = UINT32_MAX;

  std::vector<uint32_t> previous_path = {};
  auto vfs_it = vfs_filtered_files.begin();
  while(vfs_it != vfs_filtered_files.end()) {
    auto [vfs_indexed_path, vfs_path] = *vfs_it;
    //spdlog::info("rendering {} {}", vfs_path, vfs_indexed_path.size());
    // diff_point is the point of the first difference to the previous path
    uint32_t diff_point = 0;
    std::vector<uint32_t> current_path;
    for(uint32_t x = 0; x < vfs_indexed_path.size(); x++) {
      // already exceeded size of previous path -> diff point
      if(x >= previous_path.size()) {
        diff_point = x;
        break;
      }
      if(previous_path[x] != vfs_indexed_path[x]) {
        diff_point = x;
        break;
      }
      // we do not add the item that differs
      current_path.push_back(vfs_indexed_path[x]);
    }

    if(diff_point <= child_open_all) {
      child_open_all = UINT32_MAX;
    }

    //spdlog::info("diff_point {}", diff_point);
    uint32_t overflow = previous_path.size() - diff_point;
    // now go back through the previous path and close the tree nodes that were opened
    for(uint32_t x = 0; x < overflow; x++) {
      //spdlog::info("closing {} {}", x, overflow);
      ImGui::TreePop();
    }

    // now go from the diff point to the end of the indexed path and open the tree nodes
    // and update the current path
    for(uint32_t x = diff_point; x < vfs_indexed_path.size(); x++) {
      auto part_it = std::next(vfs_string_table.begin(), vfs_indexed_path[x]);
      std::string part = part_it->first;
      //spdlog::info("part '{}' x {} cur_path {}", part, x, current_path.size());
      // selectable only for last nodes
      // inside the loop as we need to not do this when one of the parents is closed
      if(x == vfs_indexed_path.size()-1) {
        // we are at the end of the path, so we need to render the selectable
        if(ImGui::Selectable(part.c_str(), selected_vfs_path == vfs_path)) {
          selected_vfs_path = vfs_path;
          ret = vfs_files[vfs_path];
        }
        if(vfs_path == selected_vfs_path) {
          ImGui::SetItemDefaultFocus();
        }
        vfs_it++;
        break;
      }
      // keyboard navigation
      if(child_open_all < x) {
        ImGui::SetNextItemOpen(true);
      }
      // node handling
      if(!ImGui::TreeNodeEx(part.c_str(), ImGuiTreeNodeFlags_NavLeftJumpsBackHere)) {
        // node is closed, so we need to advance to the next part
        // if the node is opened we do not need to do anything
        std::vector<uint32_t> next_path = current_path;
        next_path.push_back(vfs_indexed_path[x]+1);
        vfs_it = vfs_filtered_files.upper_bound(next_path);
        break;
      } else {
        // when the tree node was opened, we need to push the 
        // current part so it will be closed at a later point
        current_path.push_back(vfs_indexed_path[x]);
        // keyboard navigation
        // if the node is selected but already open, open all sub nodes
        if(!ImGui::IsItemToggledOpen() && ImGui::IsItemHovered() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) {
          child_open_all = x;
        }
      }
    }
    previous_path = current_path;
  }
  // if previous_path still contains items, we need to close them
  for(uint32_t x = 0; x < previous_path.size(); x++) {
    ImGui::TreePop();
  }

  /*
  for(auto& item : tree) {
    //spdlog::info("rendering item {}", item.part);

    if (item.value.type() == typeid(std::vector<FileTreeItem>)) {
      if(!m_search_lower.empty()) {
        bool cont = true;
        for(auto& sub_path : item.sub_paths) {
          if(str_tolower(sub_path).contains(m_search_lower)) {
            cont = false;
            break;
          }
        }
        if(cont) {
          continue;
        }
      }
      if(open_all) {
        ImGui::SetNextItemOpen(true);
      }
      if(ImGui::TreeNodeEx(item.part.c_str(), ImGuiTreeNodeFlags_NavLeftJumpsBackHere)) {
        bool child_open_all = false;
        if(!ImGui::IsItemToggledOpen() && ImGui::IsItemHovered() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) {
          child_open_all = true;
        }
        auto sub_ret = render_file_tree(std::any_cast<std::vector<FileTreeItem>>(item.value), open_all || child_open_all);
        if(!ret.has_value()) {
          ret = sub_ret;
        }

        ImGui::TreePop();
      }
    } else {
      FileMeta value = std::any_cast<FileMeta>(item.value);
      if(!str_tolower(value.vfs_path).contains(m_search_lower)) {
        continue;
      }
      if(ImGui::Selectable(item.part.c_str(), selected_vfs_path == value.vfs_path)) {
        selected_vfs_path = value.vfs_path;
        ret = value;
      }
      if(value.vfs_path == selected_vfs_path) {
        ImGui::SetItemDefaultFocus();
      }
    }
  }*/

  return ret;
}

std::optional<FileMeta> FileTree::render() {
  ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
  if(ImGui::InputText("##file_tree_search", &m_search)) {
    m_search_lower = str_tolower(m_search);
    filter_filetree();
  }

  ImGui::Checkbox(gettext("VFS Tree"), &m_tree_view);

  std::optional<FileMeta> ret = std::nullopt;
  if(m_tree_view) {
    ret = render_file_tree();
  } else {
    ImGui::BeginListBox("##Filelist", ImGui::GetContentRegionAvail());
    ret = render_file_list();
    ImGui::EndListBox();
  }
  return ret;
}
