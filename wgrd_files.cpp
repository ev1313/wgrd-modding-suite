#include "wgrd_files.h"

#include <algorithm>
#include <utility>
#include <iostream>
#include <imgui.h>

#include "file_tree.h"

#include "imgui_stdlib.h"

#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include "ndf.hpp"

#include <libintl.h>

#include <filesystem>

namespace fs = std::filesystem;
using namespace wgrd_files;

bool wgrd_files::File::imgui_call() {
  ImGui::Text("File: %s", vfs_path.c_str());
  if(ImGui::Button("save file")) {
    std::filesystem::path vfs(vfs_path);
    auto filename = vfs.filename().string();
    auto extension = ".*," + vfs.extension().string();
    IGFD::FileDialogConfig config;
    config.path = filename.c_str();
    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", extension.c_str(), config);
  }
  if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
  {
    if (ImGuiFileDialog::Instance()->IsOk())
    {
      std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
      std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

      copy_to_file(filePathName);
    }

    ImGuiFileDialog::Instance()->Close();
  }
  return true;
}

wgrd_files::File::File(std::string vfs_path, std::ifstream &f, size_t offset, size_t size)
: vfs_path(std::move(vfs_path)), file(std::move(f)), offset(offset), size(size) {
}

std::vector<char> File::get_file() {
  size_t old = file.tellg();
  file.seekg(offset);
  std::vector<char> ret;
  ret.resize(size);
  file.read(reinterpret_cast<char*>(ret.data()), size);
  file.seekg(old);
  return ret;
}

bool File::copy_to_file(std::filesystem::path path) {
  fs::create_directories(path.parent_path());

  std::ofstream of;
  of.open(path, std::ios::out | std::ios::binary | std::ios::trunc);

  if(!of.is_open()) {
    return false;
  }

  size_t old = file.tellg();
  file.seekg(offset);
  size_t end = offset+size;

  while(!file.eof() && file.tellg() < end) {
    char buffer[1024];
    size_t count = std::min(sizeof(buffer), end-file.tellg());
    file.read(buffer, count);
    of.write(buffer, count);
  }

  if(file.tellg() < offset+size) {
    file.seekg(old);
    return false;
  }

  file.seekg(old);
  return true;
}

wgrd_files::EDat::EDat(std::string vfs_path, std::ifstream &f, size_t offset, size_t size) : File(vfs_path, f, offset, size) {
  file_tree.init_from_path(vfs_path);
}

bool wgrd_files::EDat::imgui_call() {
  auto opened_files = file_tree.render("Edat");

  //for(auto& file_meta : opened_files) {
  //  files.add_file(file_meta, offset);
  //}

  ImGui::Text("EDat: %s", vfs_path.c_str());
  return true;
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
  ImGui::Text("Ess: %s", vfs_path.c_str());
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

wgrd_files::SFormat::SFormat(std::string vfs_path, std::ifstream &f, size_t offset, size_t size) : File(vfs_path, f, offset, size) {
}

bool wgrd_files::SFormat::imgui_call() {
  ImGui::Text("SFormat: %s", vfs_path.c_str());
  return true;
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
  ImGui::Text("TGV: %s", vfs_path.c_str());
  return true;
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
  ImGui::Text("PPK: %s", vfs_path.c_str());
  return true;
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
  ImGui::Text("Scenario: %s", vfs_path.c_str());
  return true;
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
  ImGui::Text("Dic: %s", vfs_path.c_str());
  return true;
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

int wgrd_files::NdfBin::render_object_list() {
  //static bool filter_topo = false;
  //ImGui::Checkbox(gettext("Filter Top Objects"), &filter_topo);
  static size_t object_count = ndfbin.get_object_count();

  static std::string class_filter = "";
  if(ImGui::InputText("##ClassFilter", &class_filter)) {
    object_count = ndfbin.get_object_count(class_filter);
  }


  static int item_current_idx = -1;
  if (ImGui::BeginListBox("##ObjectList", ImVec2(-FLT_MIN, 20*ImGui::GetTextLineHeightWithSpacing())))
  {

    ImGuiListClipper clipper;
    clipper.Begin(object_count);

    while (clipper.Step()) {
      int n = clipper.DisplayStart;

      for (int it = clipper.DisplayStart; it < clipper.DisplayEnd; ++it) {
        auto& object = ndfbin.get_object(it);

        if(!class_filter.empty() && !object.class_name.contains(class_filter)) {
          continue;
        }

        const bool is_selected = (item_current_idx == n);
        if (ImGui::Selectable(std::format("{} - {}", object.name, object.class_name).c_str(), is_selected)) {
          item_current_idx = n;
        }

        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        if (is_selected) {
          ImGui::SetItemDefaultFocus();
        }

        n+=1;
      }
    }
    ImGui::EndListBox();
  }
  return item_current_idx;
}

int wgrd_files::NdfBin::render_property_list(int object_idx) {
  if(object_idx == -1 || object_idx >= ndfbin.get_object_count()) {
    return -1;
  }

  auto& object = ndfbin.get_object(object_idx);

  {
    std::string object_name = object.name;
    ImGui::Text("Object Name: ");
    ImGui::SameLine();
    if(ImGui::InputText("##ObjectName", &object_name, ImGuiInputTextFlags_EnterReturnsTrue)) {
      auto change = std::make_unique<NdfTransactionChangeObjectName>();
      change->previous_name = object.name;
      change->name = object_name;
      ndfbin.apply_transaction(std::move(change));
    }
  }

  static int item_current_idx = -1;
  if (ImGui::BeginListBox("##PropertyList"))
  {
    int n = 0;

    for(auto& property : object.properties) {
      const bool is_selected = (item_current_idx == n);
      if(ImGui::Selectable(std::format("Property {} - {}", n, property->property_name).c_str(), is_selected)) {
        item_current_idx = n;
      }

      // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }

      n+=1;
    }
    ImGui::EndListBox();
  }

  return item_current_idx;
}

void wgrd_files::NdfBin::render_property(int object_idx, int property_idx) {
  if(object_idx == -1 || object_idx >= ndfbin.get_object_count()) {
    return;
  }
  auto& object = ndfbin.get_object(object_idx);
  if(property_idx == -1 || property_idx >= object.properties.size()) {
    return;
  }
  auto& property = object.properties.at(property_idx);

  auto transaction_opt = render_ndf_type(property);

  if(transaction_opt) {
    auto transaction_change = std::move(transaction_opt.value());
    transaction_change->object_name = object.name;
    transaction_change->property_name = property->property_name;
    spdlog::warn("Applying transaction {} {}", object.name, property->property_name);
    ndfbin.apply_transaction(std::move(transaction_change));
  }
}

std::optional<std::unique_ptr<NdfTransactionChangeProperty>> wgrd_files::NdfBin::render_ndf_type(std::unique_ptr<NDFProperty>& property) {
  int ndf_type_id = property->property_type;
  switch(ndf_type_id) {
    case NDFPropertyType::Bool: {
      ImGui::Text("Type: Boolean");
      auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyBool>&>(property);
      bool value = prop->value;
      if(ImGui::Checkbox("Value", &value)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_Bool>();
        change->value = value;
        return change;
      }
      break;
    }
    case 0x1: {
      ImGui::Text("Type: Int8");
      ImGui::Text("Value: %d", reinterpret_cast<std::unique_ptr<NDFPropertyInt8>&>(property)->value);
      break;
    }
    case 0x2: {
      ImGui::Text("Type: Int32");
      ImGui::Text("Value: %d", reinterpret_cast<std::unique_ptr<NDFPropertyInt32>&>(property)->value);
      break;
    }
    case 0x3: {
      ImGui::Text("Type: UInt32");
      ImGui::Text("Value: %d", reinterpret_cast<std::unique_ptr<NDFPropertyUInt32>&>(property)->value);
      break;
    }
    case 0x5: {
      ImGui::Text("Type: Float32");
      ImGui::Text("Value: %f", reinterpret_cast<std::unique_ptr<NDFPropertyFloat32>&>(property)->value);
      break;
    }
    case 0x6: {
      ImGui::Text("Type: Float64");
      ImGui::Text("Value: %f", reinterpret_cast<std::unique_ptr<NDFPropertyFloat64>&>(property)->value);
      break;
    }
    case NDFPropertyType::String: {
      ImGui::Text("Type: String");
      auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyString>&>(property);
      std::string value = prop->value;
      if(ImGui::InputText("Value", &value, ImGuiInputTextFlags_EnterReturnsTrue)) {
        spdlog::warn("changing string");
        auto change = std::make_unique<NdfTransactionChangeProperty_String>();
        change->value = value;
        return change;
      }
      break;
    }
    case NDFPropertyType::WideString: {
      ImGui::Text("Type: WideString");
      ImGui::Text("Type: String");
      auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyWideString>&>(property);
      std::string value = prop->value;
      if(ImGui::InputText("Value", &value, ImGuiInputTextFlags_EnterReturnsTrue)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_WideString>();
        change->value = value;
        return change;
      }
      break;
    }
    case 0x9: {
      if(property->is_object_reference()) {
        ImGui::Text("Type: ObjectReference");
        auto& p = reinterpret_cast<std::unique_ptr<NDFPropertyObjectReference>&>(property);
        ImGui::Text("Value: %s", p->object_name.c_str());
      }
      if(property->is_import_reference()) {
        ImGui::Text("Type: ImportReference");
        auto& p = reinterpret_cast<std::unique_ptr<NDFPropertyImportReference>&>(property);
        ImGui::Text("Value: %s", p->import_name.c_str());
      }
      break;
    }
    case 0xB: {
      ImGui::Text("Type: F32_vec3");
      auto& p = reinterpret_cast<std::unique_ptr<NDFPropertyF32_vec3>&>(property);
      ImGui::Text("Value: %f %f %f", p->x, p->y, p->z);
      break;
    }
    case 0xC: {
      ImGui::Text("Type: F32_vec4");
      auto& p = reinterpret_cast<std::unique_ptr<NDFPropertyF32_vec4>&>(property);
      ImGui::Text("Value: %f %f %f %f", p->x, p->y, p->z, p->w);
      break;
    }
    case 0xD: {
      ImGui::Text("Type: Color");
      auto& p = reinterpret_cast<std::unique_ptr<NDFPropertyColor>&>(property);
      ImGui::Text("Value: %d %d %d %d", p->r, p->g, p->b, p->a);
    }
    case 0xE: {
      ImGui::Text("Type: S32_vec3");
      auto& p = reinterpret_cast<std::unique_ptr<NDFPropertyS32_vec3>&>(property);
      ImGui::Text("Value: %d %d %d", p->x, p->y, p->z);
    }
    case 0x11: {
      ImGui::Text("Type: List");
      auto& p = reinterpret_cast<std::unique_ptr<NDFPropertyList>&>(property);
    }
    default:{
      ImGui::Text("Unknown type %02x", ndf_type_id);
      break;
    }
  }
  return std::nullopt;
}

bool wgrd_files::NdfBin::imgui_call() {
  ImGui::Text("NdfBin: %s", vfs_path.c_str());
  if(!ndfbin.is_parsing() && !ndfbin.is_parsed()) {
    if(ImGui::Button(gettext("Parse NDF"))) {
      fs::path path = fs::path("out/") / fs::path(vfs_path);
      copy_to_file(path);
      ndfbin.start_parsing(path);
    }
  }
  if(ndfbin.is_parsing()) {
    ImGui::Text("Parsing NDF");
    ndfbin.check_parsing();
  } else if(ndfbin.is_parsed()) {
    if(ImGui::Button(gettext("Regenerate NDF"))) {
      fs::path path = fs::path("out/") / fs::path(vfs_path);
      ndfbin.start_parsing(path);
    } else {
      int object = render_object_list();
      int property = render_property_list(object);
      render_property(object, property);
    }
  }
  return true;
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
  
  if(strncmp(magic, "EUG0", 4)) {
    return false;
  }

  if(strncmp(magic3, "CNDF", 4)) {
    return false;
  }

  return true;
}

void wgrd_files::Files::imgui_call(std::optional<FileMeta> meta) {
  ImGui::BeginChild(gettext("Files##Files"),  ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
  if(meta) {
    auto& m = meta.value();
    if(files.find(m.idx) != files.end()) {
      files[m.idx]->imgui_call();
    }
  }
  ImGui::EndChild();
}

void wgrd_files::Files::add_file(FileMeta meta, size_t file_offset) {
  if(files.find(meta.idx) != files.end()) {
    return;
  }
  std::ifstream f(meta.fs_path, std::ios::in | std::ios::binary);
  auto vfs_path = meta.vfs_path;
  size_t offset = file_offset + meta.offset;
  auto size = meta.size;

  if(EDat::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<EDat>(vfs_path, f, offset, size);
  } else
  if(Ess::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<Ess>(vfs_path, f, offset, size);
  } else
  if(SFormat::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<SFormat>(vfs_path, f, offset, size);
  } else
  if(TGV::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<TGV>(vfs_path, f, offset, size);
  } else
  if(PPK::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<PPK>(vfs_path, f, offset, size);
  } else
  if(Scenario::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<Scenario>(vfs_path, f, offset, size);
  } else
  if(NdfBin::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<NdfBin>(vfs_path, f, offset, size);
  } else {
    files[meta.idx] = std::make_unique<File>(vfs_path, f, offset, size);
  }
}
