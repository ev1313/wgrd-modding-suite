#include "file_tree.h"

#include <imgui.h>
#include <imgui_stdlib.h>


std::vector<FileMeta> file_tree(py::dict files, const std::string& vfs_path = "$", bool open_all = false) {
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
                ImGui::Text("File: %s %d %d", file.cast<std::string>().c_str(), offset, size);
                bool file_opened = ImGui::Button("Open");
                if(file_opened) {
                    ret.emplace_back(vfs_path + "/" + path.cast<std::string>(), file.cast<std::string>(), offset, size);
                }
            } else
            if (py::isinstance<py::dict>(value)) {
                file_tree(value.cast<py::dict>(), vfs_path + "/" + path.cast<std::string>(), open_all || child_open_all);
            } else {
                assert(false);
            }

            ImGui::TreePop();
        }
    }
}

std::vector<FileMeta> imgui_file_tree(const std::string& name, py::dict files) {
    py::object create_vfs_tree = py::module_::import("wgrd_cons_tools.create_vfs").attr("create_vfs_tree");
    py::object search_vfs_tree = py::module_::import("wgrd_cons_tools.create_vfs").attr("search_vfs_tree");

    static bool rebuild_tree = true;

    ImGui::Begin(name.c_str());
    static std::string search;
    rebuild_tree |= ImGui::InputText("##file_tree_search", &search);

    static py::dict vfs_tree;

    if(rebuild_tree) {
        if(!search.empty()) {
            files = search_vfs_tree(files, search);
        }
        vfs_tree = create_vfs_tree(files);
        rebuild_tree = false;
    }

    auto ret = file_tree(vfs_tree);
    ImGui::End();
    return ret;
}
