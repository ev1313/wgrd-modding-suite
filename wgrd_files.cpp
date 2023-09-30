#include "wgrd_files.h"

#include <utility>
#include <iostream>
#include <imgui.h>

#include "file_tree.h"

using namespace wgrd_files;

bool wgrd_files::File::imgui_call() {
    if(ImGui::BeginTabItem(vfs_path.c_str())) {
        ImGui::Text("File: %s", vfs_path.c_str());
        ImGui::EndTabItem();
        return true;
    }
    return false;
}

wgrd_files::File::File(std::string vfs_path, std::ifstream &f, size_t offset, size_t size)
        : vfs_path(std::move(vfs_path)), file(std::move(f)), offset(offset), size(size) {
}

wgrd_files::EDat::EDat(std::string vfs_path, std::ifstream &f, size_t offset, size_t size) : File(vfs_path, f, offset, size) {
    py::object create_vfs = py::module_::import("wgrd_cons_tools.create_vfs").attr("create_vfs");
    vfs_dict = create_vfs(py::list({py::str(vfs_path)}));
}

bool wgrd_files::EDat::imgui_call() {
    if(ImGui::BeginTabItem(vfs_path.c_str())) {
        // imgui_file_tree("Edat", vfs_dict, );

        ImGui::Text("EDat: %s", vfs_path.c_str());
        ImGui::EndTabItem();
        return true;
    }
    return false;
}

bool wgrd_files::EDat::is_file(std::string vfs_path, std::ifstream &f, size_t offset) {
    f.clear();
    f.seekg(offset);

    char magic[4];
    f.read(magic, sizeof(magic));

    f.clear();
    f.seekg(offset);

    if(!strcmp(magic, "edat")) {
        return true;
    }
    return false;
}

wgrd_files::Ess::Ess(std::string vfs_path, std::ifstream &f, size_t offset, size_t size) : File(vfs_path, f, offset, size) {
}

bool wgrd_files::Ess::imgui_call() {
    if(ImGui::BeginTabItem(vfs_path.c_str())) {
        ImGui::Text("Ess: %s", vfs_path.c_str());
        ImGui::EndTabItem();
        return true;
    }
    return false;
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

wgrd_files::SFormat::SFormat(std::string vfs_path, std::ifstream &f, size_t offset, size_t size) : File(vfs_path, f, offset, size) {
}

bool wgrd_files::SFormat::imgui_call() {
    if(ImGui::BeginTabItem(vfs_path.c_str())) {
        ImGui::Text("SFormat: %s", vfs_path.c_str());
        ImGui::EndTabItem();
        return true;
    }
    return false;
}

bool wgrd_files::SFormat::is_file(std::string vfs_path, std::ifstream &f, size_t offset) {
    (void)f;
    (void)offset;
    if(vfs_path.ends_with(".sformat")) {
        return true;
    }

    return false;
}

wgrd_files::TGV::TGV(std::string vfs_path, std::ifstream &f, size_t offset, size_t size) : File(vfs_path, f, offset, size) {
}

bool wgrd_files::TGV::imgui_call() {
    if(ImGui::BeginTabItem(vfs_path.c_str())) {
        ImGui::Text("TGV: %s", vfs_path.c_str());
        ImGui::EndTabItem();
        return true;
    }
    return false;
}

bool wgrd_files::TGV::is_file(std::string vfs_path, std::ifstream &f, size_t offset) {
    f.clear();
    f.seekg(offset);

    char magic[4];
    f.read(magic, sizeof(magic));

    f.clear();
    f.seekg(offset);

    if(vfs_path.ends_with(".tgv") && magic[0] == 0x02 && magic[1] == 0x00 && magic[2] == 0x00 && magic[3] == 0x00) {
        return true;
    }
    return false;
}

wgrd_files::PPK::PPK(std::string vfs_path, std::ifstream &f, size_t offset, size_t size) : File(vfs_path, f, offset, size) {
}

bool wgrd_files::PPK::imgui_call() {
    if(ImGui::BeginTabItem(vfs_path.c_str())) {
        ImGui::Text("PPK: %s", vfs_path.c_str());
        ImGui::EndTabItem();
        return true;
    }
    return false;
}

bool wgrd_files::PPK::is_file(std::string vfs_path, std::ifstream &f, size_t offset) {
    f.clear();
    f.seekg(offset);

    char magic[8];
    f.read(magic, sizeof(magic));

    f.clear();
    f.seekg(offset);

    if(vfs_path.ends_with(".ppk") && !strcmp(magic, "PRXYPCPC")) {
        return true;
    }
    return false;
}

wgrd_files::Scenario::Scenario(std::string vfs_path, std::ifstream &f, size_t offset, size_t size) : File(vfs_path, f, offset, size) {
}

bool wgrd_files::Scenario::imgui_call() {
    if(ImGui::BeginTabItem(vfs_path.c_str())) {
        ImGui::Text("Scenario: %s", vfs_path.c_str());
        ImGui::EndTabItem();
        return true;
    }
    return false;
}

bool wgrd_files::Scenario::is_file(std::string vfs_path, std::ifstream &f, size_t offset) {
    f.clear();
    f.seekg(offset);

    char magic[8];
    f.read(magic, sizeof(magic));

    f.clear();
    f.seekg(offset);

    if(!strcmp(magic, "SCENARIO")) {
        return true;
    }
    return false;
}

wgrd_files::Dic::Dic(std::string vfs_path, std::ifstream &f, size_t offset, size_t size) : File(vfs_path, f, offset, size) {
}

bool wgrd_files::Dic::imgui_call() {
    if(ImGui::BeginTabItem(vfs_path.c_str())) {
        ImGui::Text("Dic: %s", vfs_path.c_str());
        ImGui::EndTabItem();
        return true;
    }
    return false;
}

bool wgrd_files::Dic::is_file(std::string vfs_path, std::ifstream &f, size_t offset) {
    f.clear();
    f.seekg(offset);

    char magic[4];
    f.read(magic, sizeof(magic));

    f.clear();
    f.seekg(offset);

    if(!strcmp(magic, "TRAD")) {
        return true;
    }
    return false;
}

wgrd_files::NdfBin::NdfBin(std::string vfs_path, std::ifstream &f, size_t offset, size_t size) : File(vfs_path, f, offset, size) {
}

bool wgrd_files::NdfBin::imgui_call() {
    if(ImGui::BeginTabItem(vfs_path.c_str())) {
        ImGui::Text("NdfBin: %s", vfs_path.c_str());
        ImGui::EndTabItem();
        return true;
    }
    return false;
}

bool wgrd_files::NdfBin::is_file(std::string vfs_path, std::ifstream &f, size_t offset) {
    f.clear();
    f.seekg(offset);

    char magic[4];
    f.read(magic, sizeof(magic));
    char magic2[4];
    f.read(magic2, sizeof(magic2));
    char magic3[4];
    f.read(magic3, sizeof(magic3));

    f.clear();
    f.seekg(offset);

    if(!strcmp(magic, "EUG0") && !strcmp(magic3, "CNDF")) {
        return true;
    }
    return false;
}
