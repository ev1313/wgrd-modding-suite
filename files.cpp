#include <imgui.h>

#include "files.h"
#include <iostream>

using namespace wgrd_files;

void wgrd_files::Files::imgui_call() {
    if(ImGui::Begin("Files")) {
        ImGui::Text("Test Files");
        if (ImGui::BeginTabBar("Files Tabs", ImGuiTabBarFlags_None)) {
            for (auto &file: files) {
                file->imgui_call();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }
}

bool wgrd_files::Files::add_file(FileMeta meta) {
    auto& vfs_path = meta.vfs_path;
    auto& fs_path = meta.fs_path;
    auto& offset = meta.offset;
    auto& size = meta.size;
    std::ifstream f;
    f.open(fs_path, std::ios::binary | std::ios::in);

    if(EDat::is_file(vfs_path, f, offset)) {
        files.emplace_back(std::make_unique<EDat>(vfs_path, f, offset, size));
    } else
    if(Ess::is_file(vfs_path, f, offset)) {
        files.emplace_back(std::make_unique<Ess>(vfs_path, f, offset, size));
    } else
    if(SFormat::is_file(vfs_path, f, offset)) {
        files.emplace_back(std::make_unique<SFormat>(vfs_path, f, offset, size));
    } else
    if(TGV::is_file(vfs_path, f, offset)) {
        files.emplace_back(std::make_unique<TGV>(vfs_path, f, offset, size));
    } else
    if(PPK::is_file(vfs_path, f, offset)) {
        files.emplace_back(std::make_unique<PPK>(vfs_path, f, offset, size));
    } else
    if(Scenario::is_file(vfs_path, f, offset)) {
        files.emplace_back(std::make_unique<Scenario>(vfs_path, f, offset, size));
    } else
    if(NdfBin::is_file(vfs_path, f, offset)) {
        files.emplace_back(std::make_unique<NdfBin>(vfs_path, f, offset, size));
    } else {
        files.emplace_back(std::make_unique<File>(vfs_path, f, offset, size));
    }
    return true;
}

