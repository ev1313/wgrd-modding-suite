#include "wgrd_files.h"

#include <algorithm>
#include <utility>
#include <iostream>
#include <imgui.h>

#include "file_tree.h"

#include "imgui_stdlib.h"

#include "ImGuiFileDialog.h"
#include "ndf.hpp"

#include <libintl.h>

#include "magic_enum.hpp"

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

wgrd_files::File::File(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path)
: vfs_path(std::move(vfs_path)), file(std::move(f)), offset(offset), size(size), out_path(out_path) {
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

std::vector<char> File::get_data() {
  std::vector<char> ret;
  ret.resize(size);

  size_t old = file.tellg();
  file.seekg(offset);

  file.read(ret.data(), size);

  file.seekg(old);
  return std::move(ret);
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

wgrd_files::EDat::EDat(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path) : File(vfs_path, f, offset, size, out_path) {
}

bool wgrd_files::EDat::imgui_call() {
  auto opened_files = file_tree.render();

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

wgrd_files::Ess::Ess(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path) : File(vfs_path, f, offset, size, out_path) {
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

wgrd_files::SFormat::SFormat(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path) : File(vfs_path, f, offset, size, out_path) {
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

wgrd_files::TGV::TGV(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path) : File(vfs_path, f, offset, size, out_path) {
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

wgrd_files::PPK::PPK(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path) : File(vfs_path, f, offset, size, out_path) {
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

wgrd_files::Scenario::Scenario(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path) : File(vfs_path, f, offset, size, out_path) {
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

wgrd_files::Dic::Dic(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path) : File(vfs_path, f, offset, size, out_path) {
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

wgrd_files::NdfBin::NdfBin(std::string vfs_path, std::ifstream &f, size_t offset, size_t size, fs::path out_path) : File(vfs_path, f, offset, size, out_path) {
}

std::string wgrd_files::NdfBin::render_object_list() {
  if(ImGui::BeginTable("filters", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
    ImGui::TableSetupColumn(gettext("Filter"), ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn(gettext("Action"), ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableNextColumn();
    ImGui::Text("Object Filter: ");
    ImGui::TableNextColumn();
    if(ImGui::InputText("##ObjectFilter", &object_filter)) {
      object_count_changed = true;
    }
    ImGui::TableNextColumn();
    ImGui::Text("Class Filter: ");
    ImGui::TableNextColumn();
    if(ImGui::InputText("##ClassFilter", &class_filter)) {
      object_count_changed = true;
    }
    ImGui::TableNextColumn();
    //static bool filter_topo = false;
    //ImGui::Checkbox(gettext("Filter Top Objects"), &filter_topo);
    ImGui::EndTable();
  }

  if(object_count_changed) {
    object_list.clear();
    object_list = ndfbin.filter_objects(object_filter, class_filter);
    item_current_idx = -1;
    object_count_changed = false;
  }

  if(item_current_idx >= object_list.size()) {
    item_current_idx = -1;
  }

  if (ImGui::BeginListBox("##ObjectList", ImVec2(-FLT_MIN, 20*ImGui::GetTextLineHeightWithSpacing())))
  {
    ImGuiListClipper clipper;
    clipper.Begin(object_list.size());

    while (clipper.Step()) {
      for(int it = clipper.DisplayStart; it < clipper.DisplayEnd; it++) {
        const auto& object = ndfbin.get_object(object_list[it]);

        const bool is_selected = (item_current_idx == it);
        if (ImGui::Selectable(std::format("{} - {}", object.name, object.class_name).c_str(), is_selected)) {
          item_current_idx = it;
        }

        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        if (is_selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
    }
    ImGui::EndListBox();
  }

  if(item_current_idx == -1) {
    return "";
  }

  std::string object_name = object_list[item_current_idx];

  if(!object_name.empty() && ndfbin.contains_object(object_name)){
    auto& object = ndfbin.get_object(object_name);

    ImGui::BeginTable("object_prop_table", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn(gettext("##OptionName"), ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn(gettext("##EditField"), ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableNextColumn();
    ImGui::Text("Object Name: ");
    ImGui::TableNextColumn();
    if(ImGui::InputText("##ObjectName", &object_name, ImGuiInputTextFlags_EnterReturnsTrue)) {
      auto change = std::make_unique<NdfTransactionChangeObjectName>();
      change->previous_name = object.name;
      change->name = object_name;
      ndfbin.apply_transaction(std::move(change));
    }
    ImGui::TableNextColumn();
    ImGui::Text("Export Path: ");
    ImGui::TableNextColumn();
    std::string export_path = object.export_path;
    if(ImGui::InputText("##ExportPath", &export_path, ImGuiInputTextFlags_EnterReturnsTrue)) {
      auto change = std::make_unique<NdfTransactionChangeObjectExportPath>();
      change->object_name = object.name;
      change->previous_export_path = object.export_path;
      change->export_path = export_path;
      ndfbin.apply_transaction(std::move(change));
    }
    ImGui::TableNextColumn();
    ImGui::Text("Is Top Object: ");
    ImGui::TableNextColumn();
    bool is_top_object = object.is_top_object;
    if(ImGui::Checkbox("##IsTopObject", &is_top_object)) {
      auto change = std::make_unique<NdfTransactionChangeObjectTopObject>();
      change->object_name = object.name;
      change->top_object = is_top_object;
      ndfbin.apply_transaction(std::move(change));
    }

    ImGui::EndTable();

    if(ImGui::Button(gettext("Remove Object"))) {
      auto change = std::make_unique<NdfTransactionRemoveObject>();
      change->object_name = object.name;
      ndfbin.apply_transaction(std::move(change));
      object_count_changed = true;
    }
    ImGui::SameLine();
    if(ImGui::Button(gettext("Copy Object"))) {
      auto change = std::make_unique<NdfTransactionCopyObject>();
      change->object_name = object.name;
      change->new_object_name = object.name + "_copy";
      while(ndfbin.contains_object(change->new_object_name)) {
        change->new_object_name += "_copy";
      }
      ndfbin.apply_transaction(std::move(change));
      object_count_changed = true;
    }
  }

  return object_name;
}

std::string wgrd_files::NdfBin::render_property_list(std::string object_name) {
  if(object_name.empty() || !ndfbin.contains_object(object_name)){
    return "";
  }
  auto& object = ndfbin.get_object(object_name);

  if(ImGui::TreeNodeEx(gettext("Object Properties"), ImGuiTreeNodeFlags_DefaultOpen)){
    ImGui::BeginTable("property_table", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable | ImGuiTableFlags_Resizable);
    ImGui::TableSetupColumn(gettext("Name"), ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn(gettext("Type"), ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn(gettext("Value"), ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();
    for(const auto& property : object.properties) {
      ImGui::TableNextColumn();
      ImGui::Text("%s", property->property_name.c_str());
      ImGui::TableNextColumn();
      ImGui::Text("%s(0x%02x)", std::string(magic_enum::enum_name((NDFPropertyType)property->property_type)).c_str(), property->property_type);
      ImGui::TableNextColumn();
      render_property(object_name, property->property_name);
    }
    ImGui::EndTable();
    ImGui::TreePop();
  }

  return "";
}

void wgrd_files::NdfBin::render_property(std::string object_name, std::string property_name) {
  auto& object = ndfbin.get_object(object_name);
  auto& property_idx = object.property_map.at(property_name);
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
  const float drag_speed = 0.2f;

  int ndf_type_id = property->property_type;
  switch(ndf_type_id) {
    case NDFPropertyType::Bool: {
      auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyBool>&>(property);
      bool value = prop->value;
      if(ImGui::Checkbox(std::format("##bool_prop_{}", property->property_name).c_str(), &value)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_Bool>();
        change->value = value;
        return change;
      }
      break;
    }
    case NDFPropertyType::UInt8: {
      auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyUInt8>&>(property);
      uint8_t val = prop->value;
      uint8_t min = 0;
      uint8_t max = UINT8_MAX;
      if(ImGui::DragScalar(std::format("##u8_prop_{}", property->property_name).c_str(), ImGuiDataType_U8, &val, drag_speed, &min, &max)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_UInt8>();
        change->value = val;
        return change;
      }
      break;
    }
    case NDFPropertyType::Int16: {
      auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyInt16>&>(property);
      int16_t val = prop->value;
      int16_t min = INT16_MIN;
      int16_t max = INT16_MAX;
      if(ImGui::DragScalar(std::format("##s16_prop_{}", property->property_name).c_str(), ImGuiDataType_S16, &val, drag_speed, &min, &max)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_Int16>();
        change->value = val;
        return change;
      }
      break;
    }
    case NDFPropertyType::UInt16: {
      auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyUInt16>&>(property);
      uint16_t val = prop->value;
      uint16_t min = 0;
      uint16_t max = UINT16_MAX;
      if(ImGui::DragScalar(std::format("##u16_prop_{}", property->property_name).c_str(), ImGuiDataType_U16, &val, drag_speed, &min, &max)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_UInt16>();
        change->value = val;
        return change;
      }
      break;
    }
    case NDFPropertyType::Int32: {
      auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyInt32>&>(property);
      int32_t val = prop->value;
      int32_t min = INT32_MIN;
      int32_t max = INT32_MAX;
      if(ImGui::DragScalar(std::format("##s32_prop_{}", property->property_name).c_str(), ImGuiDataType_S32, &val, drag_speed, &min, &max)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_Int32>();
        change->value = val;
        return change;
      }
      break;
    }
    case NDFPropertyType::UInt32: {
      auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyUInt32>&>(property);
      uint32_t val = prop->value;
      uint32_t min = 0;
      uint32_t max = UINT32_MAX;
      if(ImGui::DragScalar(std::format("##u32_prop_{}", property->property_name).c_str(), ImGuiDataType_U32, &val, drag_speed, &min, &max)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_UInt32>();
        change->value = val;
        return change;
      }
      break;
    }
    case NDFPropertyType::Float32: {
      auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyFloat32>&>(property);
      float val = prop->value;
      if(ImGui::DragFloat(std::format("##f32_prop_{}", property->property_name).c_str(), &val, drag_speed)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_Float32>();
        change->value = val;
        return change;
      }
      break;
    }
    case NDFPropertyType::Float64: {
      auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyFloat64>&>(property);
      double val = prop->value;
      if(ImGui::InputDouble(std::format("##f64_prop_{}", property->property_name).c_str(), &val)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_Float64>();
        change->value = val;
        return change;
      }
      break;
    }
    case NDFPropertyType::String: {
      auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyString>&>(property);
      std::string value = prop->value;
      if(ImGui::InputText(std::format("##string_prop_{}", property->property_name).c_str(), &value, ImGuiInputTextFlags_EnterReturnsTrue)) {
        spdlog::warn("changing string");
        auto change = std::make_unique<NdfTransactionChangeProperty_String>();
        change->value = value;
        return change;
      }
      break;
    }
    case NDFPropertyType::WideString: {
      auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyWideString>&>(property);
      std::string value = prop->value;
      if(ImGui::InputText(std::format("##widestring_prop_{}", property->property_name).c_str(), &value, ImGuiInputTextFlags_EnterReturnsTrue)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_WideString>();
        change->value = value;
        return change;
      }
      break;
    }
    case NDFPropertyType::PathReference: {
      auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyPathReference>&>(property);
      std::string path = prop->path;
      if(ImGui::InputText(std::format("##string_prop_{}", property->property_name).c_str(), &path, ImGuiInputTextFlags_EnterReturnsTrue)) {
        spdlog::warn("changing string");
        auto change = std::make_unique<NdfTransactionChangeProperty_PathReference>();
        change->path = path;
        return change;
      }
      break;
    }
    case NDFPropertyType::ObjectReference: {
      if(property->is_object_reference()) {
        auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyObjectReference>&>(property);
        std::string value = prop->object_name;
        if(ImGui::InputText(std::format("##objref_prop_{}", property->property_name).c_str(), &value, ImGuiInputTextFlags_EnterReturnsTrue)) {
          auto change = std::make_unique<NdfTransactionChangeProperty_ObjectReference>();
          change->value = value;
          return change;
        }
        ImGui::SameLine();
        if(!ndfbin.contains_object(value)) {
          ImGui::Text("Object %s does not exist", value.c_str());
        } else {
          if(ImGui::Button(gettext("Jump"))) {
            // FIXME: maybe reset object / class filters and then just use normal way of finding it?
            int idx = -1;
            for(const auto& [i, obj] : object_list | std::views::enumerate) {
              if(obj == value) {
                idx = i;
                break;
              }
            }
            if(idx != -1) {
              item_current_idx = idx;
            }
          }
        }
        break;
      }
      if(property->is_import_reference()) {
        auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyImportReference>&>(property);
        std::string value = prop->import_name;
        if(ImGui::InputText(std::format("##imprref_prop_{}", property->property_name).c_str(), &value, ImGuiInputTextFlags_EnterReturnsTrue)) {
          auto change = std::make_unique<NdfTransactionChangeProperty_ImportReference>();
          change->value = value;
          return change;
        }
        break;
      }
      break;
    }
    case NDFPropertyType::F32_vec2: {
      auto& p = reinterpret_cast<std::unique_ptr<NDFPropertyF32_vec2>&>(property);
      float val[2] = {p->x, p->y};
      if(ImGui::InputFloat3(std::format("##f32_vec2_prop_{}", property->property_name).c_str(), val)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_F32_vec2>();
        change->x = val[0];
        change->y = val[1];
        return change;
      }
      break;
    }
    case NDFPropertyType::F32_vec3: {
      auto& p = reinterpret_cast<std::unique_ptr<NDFPropertyF32_vec3>&>(property);
      float val[3] = {p->x, p->y, p->z};
      if(ImGui::InputFloat3(std::format("##f32_vec3_prop_{}", property->property_name).c_str(), val)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_F32_vec3>();
        change->x = val[0];
        change->y = val[1];
        change->z = val[2];
        return change;
      }
      break;
    }
    case NDFPropertyType::F32_vec4: {
      auto& p = reinterpret_cast<std::unique_ptr<NDFPropertyF32_vec4>&>(property);
      float val[4] = {p->x, p->y, p->z, p->w};
      if(ImGui::InputFloat3(std::format("##f32_vec4_prop_{}", property->property_name).c_str(), val)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_F32_vec4>();
        change->x = val[0];
        change->y = val[1];
        change->z = val[2];
        change->w = val[3];
        return change;
      }
      break;
    }
    case NDFPropertyType::Color: {
      if(ImGui::TreeNodeEx(gettext("Color"), ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& p = reinterpret_cast<std::unique_ptr<NDFPropertyColor>&>(property);
        uint8_t val[4] = {p->r, p->g, p->b, p->a};
        float fval[4] = {p->r / 255.0f, p->g / 255.0f, p->b / 255.0f, p->a / 255.0f};
        if(ImGui::ColorPicker4(std::format("##color_prop_{}", property->property_name).c_str(), fval)) {
          val[0] = fval[0] * 255;
          val[1] = fval[1] * 255;
          val[2] = fval[2] * 255;
          val[3] = fval[3] * 255;
          auto change = std::make_unique<NdfTransactionChangeProperty_Color>();
          change->r = val[0];
          change->g = val[1];
          change->b = val[2];
          change->a = val[3];
          return change;
        }
        ImGui::TreePop();
      }
      break;
    }
    case NDFPropertyType::NDFGUID: {
      auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyGUID>&>(property);
      std::string value = prop->guid;
      if(ImGui::InputText(std::format("##guid_prop_{}", property->property_name).c_str(), &value, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_GUID>();
        change->guid = value;
        return change;
      }
      break;
    }
    case NDFPropertyType::LocalisationHash: {
      auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyLocalisationHash>&>(property);
      std::string value = prop->hash;
      if(ImGui::InputText(std::format("##lochash_prop_{}", property->property_name).c_str(), &value, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_LocalisationHash>();
        change->hash = value;
        return change;
      }
      break;
    }
    case NDFPropertyType::Hash: {
      auto& prop = reinterpret_cast<std::unique_ptr<NDFPropertyHash>&>(property);
      std::string value = prop->hash;
      if(ImGui::InputText(std::format("##hash_prop_{}", property->property_name).c_str(), &value, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_Hash>();
        change->hash = value;
        return change;
      }
      break;
    }
    case NDFPropertyType::S32_vec2: {
      auto& p = reinterpret_cast<std::unique_ptr<NDFPropertyS32_vec2>&>(property);
      int32_t val[2] = {p->x, p->y};
      if(ImGui::InputInt2(std::format("##f32_vec2_prop_{}", property->property_name).c_str(), val)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_S32_vec2>();
        change->x = val[0];
        change->y = val[1];
        return change;
      }
      break;
    }
    case NDFPropertyType::S32_vec3: {
      auto& p = reinterpret_cast<std::unique_ptr<NDFPropertyS32_vec3>&>(property);
      int32_t val[3] = {p->x, p->y, p->z};
      if(ImGui::InputInt3(std::format("##f32_vec3_prop_{}", property->property_name).c_str(), val)) {
        auto change = std::make_unique<NdfTransactionChangeProperty_S32_vec3>();
        change->x = val[0];
        change->y = val[1];
        change->z = val[2];
        return change;
      }
      break;
    }
    case NDFPropertyType::List: {
      std::optional<std::unique_ptr<NdfTransactionChangeProperty>> ret = std::nullopt;
      auto& p = reinterpret_cast<std::unique_ptr<NDFPropertyList>&>(property);
      if(ImGui::BeginTable(std::format("list_table_{}", property->property_name).c_str(), 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn(gettext("List Index"), ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn(gettext("List Items"), ImGuiTableColumnFlags_WidthStretch);
        for(const auto& [idx, item] : p->values | std::views::enumerate) {
          ImGui::TableNextColumn();
          ImGui::Text("%d", (unsigned int)idx);
          ImGui::TableNextColumn();
          item->property_name = std::to_string(idx);
          auto change = render_ndf_type(item);
          if(change) {
            auto list_change = std::make_unique<NdfTransactionChangeProperty_ChangeListItem>();
            list_change->index = idx;
            list_change->change = std::move(change.value());
            ret = std::move(list_change);
          }
        }
        ImGui::EndTable();
      }
      return ret;
      break;
    }
    case NDFPropertyType::Map: {
      std::optional<std::unique_ptr<NdfTransactionChangeProperty>> ret = std::nullopt;
      auto& p = reinterpret_cast<std::unique_ptr<NDFPropertyMap>&>(property);
      if(ImGui::BeginTable(std::format("map_table_{}", property->property_name).c_str(), 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn(gettext("Map Key"), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(gettext("Map Value"), ImGuiTableColumnFlags_WidthStretch);
        for(const auto& [idx, item] : p->values | std::views::enumerate) {
          ImGui::TableNextColumn();
          {
            item.first->property_name = "Key_" + std::to_string(idx);
            auto change = render_ndf_type(item.first);
            if(change) {
              auto key_change = std::make_unique<NdfTransactionChangeProperty_ChangeMapItem>();
              key_change->index = idx;
              key_change->key = true;
              key_change->change = std::move(change.value());
              ret = std::move(key_change);
            }
          }
          ImGui::TableNextColumn();
          {
            item.second->property_name = "Value_" + std::to_string(idx);
            auto change = render_ndf_type(item.second);
            if(change) {
              auto value_change = std::make_unique<NdfTransactionChangeProperty_ChangeMapItem>();
              value_change->index = idx;
              value_change->key = false;
              value_change->change = std::move(change.value());
              ret = std::move(value_change);
            }
          }
        }
        ImGui::EndTable();
        return ret;
      }
      break;
    }
    case NDFPropertyType::Pair: {
      std::optional<std::unique_ptr<NdfTransactionChangeProperty>> ret = std::nullopt;
      auto& p = reinterpret_cast<std::unique_ptr<NDFPropertyPair>&>(property);
      if(ImGui::BeginTable(std::format("pair_table_{}", property->property_name).c_str(), 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn(gettext("First"), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(gettext("Second"), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextColumn();
        {
          p->first->property_name = "First";
          auto change = render_ndf_type(p->first);
          if(change) {
            auto first_change = std::make_unique<NdfTransactionChangeProperty_ChangePairItem>();
            first_change->first = true;
            first_change->change = std::move(change.value());
            ret = std::move(first_change);
          }
        }
        ImGui::TableNextColumn();
        {
          p->second->property_name = "Second";
          auto change = render_ndf_type(p->second);
          if(change) {
            auto second_change = std::make_unique<NdfTransactionChangeProperty_ChangePairItem>();
            second_change->first = false;
            second_change->change = std::move(change.value());
            ret = std::move(second_change);
          }
        }
        ImGui::EndTable();
      }
      return ret;
      break;
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
      ndfbin.start_parsing(vfs_path, get_data());
    }
  }
  if(ndfbin.is_parsing()) {
    ImGui::Text("Parsing NDF");
    ndfbin.check_parsing();
  } else if(ndfbin.is_parsed()) {
    if(ImGui::Button(gettext("Regenerate NDF"))) {
      ndfbin.start_parsing(vfs_path, get_data());
    }
    ImGui::SameLine();
    if(ImGui::Button(gettext("Save XML"))) {
      ndfbin.save_ndf_xml_to_file(out_path / "xml" / fs::path(vfs_path).replace_extension(".ndf.xml"));
    }
    ImGui::SameLine();
    if(ImGui::Button(gettext("Save ndfbin"))) {
      ndfbin.save_ndfbin_to_file(out_path / "bin" / vfs_path);
    }
  }
  if(ndfbin.is_parsed()) {
    auto object = render_object_list();
    auto property = render_property_list(object);
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
  ImGui::BeginChild(gettext("Files"),  ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
  if(meta) {
    auto& m = meta.value();
    if(files.find(m.idx) != files.end()) {
      files[m.idx]->imgui_call();
    }
  }
  ImGui::EndChild();
}

void wgrd_files::Files::add_file(fs::path out_path, FileMeta meta, size_t file_offset) {
  if(files.find(meta.idx) != files.end()) {
    return;
  }
  std::ifstream f(meta.fs_path, std::ios::in | std::ios::binary);
  auto vfs_path = meta.vfs_path;
  size_t offset = file_offset + meta.offset;
  auto size = meta.size;

  if(EDat::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<EDat>(vfs_path, f, offset, size, out_path);
  } else
  if(Ess::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<Ess>(vfs_path, f, offset, size, out_path);
  } else
  if(SFormat::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<SFormat>(vfs_path, f, offset, size, out_path);
  } else
  if(TGV::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<TGV>(vfs_path, f, offset, size, out_path);
  } else
  if(PPK::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<PPK>(vfs_path, f, offset, size, out_path);
  } else
  if(Scenario::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<Scenario>(vfs_path, f, offset, size, out_path);
  } else
  if(NdfBin::is_file(vfs_path, f, offset)) {
    files[meta.idx] = std::make_unique<NdfBin>(vfs_path, f, offset, size, out_path);
  } else {
    files[meta.idx] = std::make_unique<File>(vfs_path, f, offset, size, out_path);
  }
}
