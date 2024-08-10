#include "ndfbin.hpp"

using namespace wgrd_files;

#include <imgui.h>
#include "misc/cpp/imgui_stdlib.h"

#include "magic_enum.hpp"

wgrd_files::NdfBin::NdfBin(FileMeta meta, fs::path out_path) : File(meta, out_path) {
}

std::string wgrd_files::NdfBin::render_object_list() {
  if(ImGui::BeginTable("filters", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
    ImGui::TableSetupColumn(gettext("Filter"), ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn(gettext("Action"), ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableNextColumn();
    ImGui::Text("Object Filter: ");
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if(ImGui::InputText("##ObjectFilter", &object_filter)) {
      object_count_changed = true;
    }
    ImGui::TableNextColumn();
    ImGui::Text("Class Filter: ");
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if(ImGui::InputText("##ClassFilter", &class_filter)) {
      object_count_changed = true;
    }
    ImGui::TableNextColumn();
    //static bool filter_topo = false;
    //ImGui::Checkbox(gettext("Filter Top Objects"), &filter_topo);
    ImGui::EndTable();
  }

  if(object_count_changed) {
    fill_class_list();
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
          std::string object_name = object_list[item_current_idx];
          if(!object_name.empty()) {
            open_object_windows.insert({object_name, true});
          }
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

  return object_name;
}

void wgrd_files::NdfBin::fill_class_list() {
  class_list.clear();
  for(auto& object_name : ndfbin.filter_objects("", "")) {
    auto& object = ndfbin.get_object(object_name);
    auto class_it = class_list.find(object.class_name);
    if(class_it == class_list.end()) {
      class_list.emplace(object.class_name, Class());
      class_it = class_list.find(object.class_name);
    }
    class_it->second.objects.push_back(object_name);
    for(auto& property : object.properties) {
      auto prop_it = class_it->second.properties.find(property->property_name);
      if(prop_it == class_it->second.properties.end()) {
        class_it->second.properties.emplace(property->property_name, Property());
        prop_it = class_it->second.properties.find(property->property_name);
      }
      prop_it->second.values[property->as_string()].insert(object_name);
    }
  }
}

std::string wgrd_files::NdfBin::render_class_list() {
  ImGui::Text(gettext("Class List:"));
  std::string ret = "";
  if(ImGui::BeginListBox("##Class List", ImVec2(-FLT_MIN, 20*ImGui::GetTextLineHeightWithSpacing()))) {
    for(auto& [class_name, class_] : class_list) {
      if(!class_filter.empty() && !class_name.contains(class_filter)) {
        continue;
      }
      if(ImGui::Selectable(class_name.c_str(), selected_class == class_name)) {
        selected_class = class_name;
        ret = class_name;
        open_class_windows.insert({class_name, true});
      }

      if(selected_class == class_name) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndListBox();
  }
  
  return ret;
}

void wgrd_files::NdfBin::render_classes() {
  for(auto& [class_name, p_open] : open_class_windows) {
    if(!class_list.contains(class_name)) {
      spdlog::error("Class {} not found", class_name);
      continue;
    }
    auto& class_ = class_list.at(class_name);

    ImGui::SetNextWindowSize(ImVec2(600, 800), ImGuiCond_FirstUseEver);
    if(ImGui::Begin(class_name.c_str(), &p_open)) {
      // object list
      if (ImGui::BeginListBox("##ClassObjectList", ImVec2(-FLT_MIN, 15*ImGui::GetTextLineHeightWithSpacing())))
      {
        ImGuiListClipper clipper;
        clipper.Begin(class_.objects.size());

        while (clipper.Step()) {
          for(int it = clipper.DisplayStart; it < clipper.DisplayEnd; it++) {
            const auto& object = ndfbin.get_object(class_.objects[it]);

            if (ImGui::Selectable(object.name.c_str(), false)) {
              open_object_windows.insert({object.name, true});
            }
          }
        }
        ImGui::EndListBox();
      }
      // property list
      if(ImGui::BeginTable("class_prop_table", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn(gettext("Property Name"), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(gettext("Available Property Values"), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for(auto& [property_name, property] : class_.properties) {
          ImGui::TableNextColumn();
          ImGui::Text("%s", property_name.c_str());
          ImGui::TableNextColumn();
          if(ImGui::BeginTable("class_prop_value_table", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable | ImGuiTableFlags_Resizable)) {
            ImGui::TableSetupColumn(gettext("Value"), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn(gettext("Count"), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn(gettext("Objects"), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for(auto& [value, objects] : property.values) {
              ImGui::TableNextColumn();
              ImGui::Text("%s", value.c_str());
              ImGui::TableNextColumn();
              ImGui::Text("%lu", objects.size());
              ImGui::TableNextColumn();
              auto objects_it = objects | std::views::join_with(',');
              std::string objects_str = std::string(objects_it.begin(), objects_it.end());
              ImGui::Text("%s", objects_str.c_str());
            }
            
            ImGui::EndTable();
          }
        }
        ImGui::EndTable();
      }
    }
    ImGui::End();
  }
  for(auto it = open_class_windows.begin(); it != open_class_windows.end();) {
    if(!it->second) {
      it = open_class_windows.erase(it);
    } else {
      ++it;
    }
  }
}

std::optional<std::unique_ptr<NdfTransaction>> wgrd_files::NdfBin::render_object_info(std::string object_name) {
  std::optional<std::unique_ptr<NdfTransaction>> ret = std::nullopt;
  if(object_name.empty() || !ndfbin.contains_object(object_name)){
    return ret;
  }
  auto& object = ndfbin.get_object(object_name);

  if(ImGui::BeginTable("object_prop_table", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
    ImGui::TableSetupColumn(gettext("##OptionName"), ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn(gettext("##EditField"), ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableNextColumn();
    ImGui::Text("Class Name: ");
    ImGui::TableNextColumn();
    ImGui::Text("%s", object.class_name.c_str());
    ImGui::TableNextColumn();
    ImGui::Text("Object Name: ");
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    std::string changed_object_name = object_name;
    if(ImGui::InputText("##ObjectName", &changed_object_name, ImGuiInputTextFlags_EnterReturnsTrue)) {
      auto change = std::make_unique<NdfTransactionChangeObjectName>();
      change->object_name = object.name;
      change->name = changed_object_name;
      ret = std::move(change);
      // FIXME: update name in open_object_windows not here? Breaks, when transaction is not done...
      open_object_windows[changed_object_name] = true;
    }
    ImGui::TableNextColumn();
    ImGui::Text("Export Path: ");
    ImGui::TableNextColumn();
    std::string export_path = object.export_path;
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if(ImGui::InputText("##ExportPath", &export_path, ImGuiInputTextFlags_EnterReturnsTrue)) {
      auto change = std::make_unique<NdfTransactionChangeObjectExportPath>();
      change->object_name = object.name;
      change->previous_export_path = object.export_path;
      change->export_path = export_path;
      ret = std::move(change);
    }
    ImGui::TableNextColumn();
    ImGui::Text("Is Top Object: ");
    ImGui::TableNextColumn();
    bool is_top_object = object.is_top_object;
    if(ImGui::Checkbox("##IsTopObject", &is_top_object)) {
      auto change = std::make_unique<NdfTransactionChangeObjectTopObject>();
      change->object_name = object.name;
      change->top_object = is_top_object;
      ret = std::move(change);
    }

    ImGui::EndTable();
  }

  if(ImGui::Button(gettext("Remove Object"))) {
    auto change = std::make_unique<NdfTransactionRemoveObject>();
    change->object_name = object.name;
    ret = std::move(change);
  }
  ImGui::SameLine();
  if(ImGui::Button(gettext("Copy Object"))) {
    auto change = std::make_unique<NdfTransactionCopyObject>();
    change->object_name = object.name;
    change->new_object_name = object.name + "_copy";
    while(ndfbin.contains_object(change->new_object_name)) {
      change->new_object_name += "_copy";
    }
    ret = std::move(change);
  }
  return ret;
}

void wgrd_files::NdfBin::render_property_list(std::string object_name) {
  // in case Remove Object was pressed
  if(object_name.empty() || !ndfbin.contains_object(object_name)){
    return;
  }
  auto& object = ndfbin.get_object(object_name);

  if(ImGui::BeginTable("property_table", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable | ImGuiTableFlags_Resizable)) {
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
  }
}

void wgrd_files::NdfBin::render_property(std::string object_name, std::string property_name) {
  spdlog::debug("rendering property {} {}", object_name, property_name);
  auto& object = ndfbin.get_object(object_name);
  auto& property_idx = object.property_map.at(property_name);
  auto& property = object.properties.at(property_idx);

  auto transaction_opt = render_ndf_type(property);

  if(transaction_opt) {
    auto transaction_change = std::move(transaction_opt.value());
    transaction_change->object_name = object.name;
    transaction_change->property_name = property->property_name;
    spdlog::debug("Applying transaction {} {}", object.name, property->property_name);
    ndfbin.apply_transaction(std::move(transaction_change));
    m_is_changed = true;
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
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
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
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
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
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
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
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
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
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
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
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
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
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
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
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
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
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
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
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
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
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
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
      if(ImGui::BeginTable(std::format("list_table_{}", property->property_name).c_str(), 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn(gettext("List Index"), ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn(gettext("List Items"), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##ItemButtons", ImGuiTableColumnFlags_WidthStretch);
        for(const auto& [idx, item] : p->values | std::views::enumerate) {
          ImGui::PushID(idx);
          ImGui::TableNextColumn();
          ImGui::Text("%d", (unsigned int)idx);
          ImGui::TableNextColumn();
          item->property_name = "ListItem";
          auto change = render_ndf_type(item);
          if(change) {
            auto list_change = std::make_unique<NdfTransactionChangeProperty_ChangeListItem>();
            list_change->index = idx;
            list_change->change = std::move(change.value());
            ret = std::move(list_change);
          }
          ImGui::TableNextColumn();
          if(ImGui::Button(gettext("Add"))) {
            auto list_change = std::make_unique<NdfTransactionChangeProperty_AddListItem>();
            list_change->index = idx;
            list_change->value = item->get_copy();
            spdlog::debug("Adding item to list @{}", idx);
            ret = std::move(list_change);
          }
          ImGui::SameLine();
          if(ImGui::Button(gettext("Remove"))) {
            auto list_change = std::make_unique<NdfTransactionChangeProperty_RemoveListItem>();
            list_change->index = idx;
            spdlog::debug("Removing item from list @{}", idx);
            ret = std::move(list_change);
          }
          ImGui::PopID();
        }
        ImGui::EndTable();
      }
      return ret;
      break;
    }
    case NDFPropertyType::Map: {
      std::optional<std::unique_ptr<NdfTransactionChangeProperty>> ret = std::nullopt;
      auto& p = reinterpret_cast<std::unique_ptr<NDFPropertyMap>&>(property);
      if(ImGui::BeginTable(std::format("map_table_{}", property->property_name).c_str(), 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn(gettext("Map Key"), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(gettext("Map Value"), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##ItemButtons", ImGuiTableColumnFlags_WidthStretch);
        for(const auto& [idx, item] : p->values | std::views::enumerate) {
          ImGui::PushID(idx);
          ImGui::TableNextColumn();
          {
            item.first->property_name = "Key";
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
            item.second->property_name = "Value";
            auto change = render_ndf_type(item.second);
            if(change) {
              auto value_change = std::make_unique<NdfTransactionChangeProperty_ChangeMapItem>();
              value_change->index = idx;
              value_change->key = false;
              value_change->change = std::move(change.value());
              ret = std::move(value_change);
            }
          }
          ImGui::TableNextColumn();
          if(ImGui::Button(gettext("Add"))) {
            auto map_change = std::make_unique<NdfTransactionChangeProperty_AddMapItem>();
            map_change->index = idx;
            map_change->value = std::make_pair(item.first->get_copy(), item.second->get_copy());
            spdlog::debug("Adding item to map @{}", idx);
            ret = std::move(map_change);
          }
          ImGui::SameLine();
          if(ImGui::Button(gettext("Remove"))) {
            auto map_change = std::make_unique<NdfTransactionChangeProperty_RemoveMapItem>();
            map_change->index = idx;
            spdlog::debug("Removing item from map @{}", idx);
            ret = std::move(map_change);
          }
          ImGui::PopID();
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

bool wgrd_files::NdfBin::render() {
  ImGuiWindowFlags wndflags = ImGuiWindowFlags_None;
  if(is_changed()) {
    wndflags |= ImGuiWindowFlags_UnsavedDocument;
  }
  ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
  ImGui::Begin(vfs_path.c_str(), &window_opened, wndflags);
  if(!ndfbin.is_parsing() && !ndfbin.is_parsed()) {
    if(ImGui::Button(gettext("Parse NDF"))) {
      ndfbin.start_parsing(vfs_path, get_data(), xml_path);
    }
  }
  if(ndfbin.is_parsing()) {
    ImGui::Text("Parsing NDF");
    ndfbin.check_parsing();
  } else if(ndfbin.is_parsed()) {
    if(ImGui::Button(gettext("Regenerate NDF"))) {
      ndfbin.start_parsing(vfs_path, get_data(), xml_path);
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
  std::string object = "";
  if(ndfbin.is_parsed()) {
    render_object_list();
    render_class_list();
    render_classes();
  }
  ImGui::End();

  if(ndfbin.is_parsed()) {
    std::optional<std::unique_ptr<NdfTransaction>> change;
    for(auto& [object_name, p_open] : open_object_windows) {
      ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);

      bool* ptr = &p_open;

      if(object == object_name) {
        ptr = nullptr;
      }

      if(ImGui::Begin(object_name.c_str(), ptr)) {
        change = render_object_info(object_name);
        render_property_list(object_name);
      }
      ImGui::End();

      if(!p_open) {
        open_object_windows.erase(object_name);
      }
    }

    if(change) {
      std::string object_name = change.value()->object_name;
      ndfbin.apply_transaction(std::move(change.value()));
      // also gets triggered when changing top object, but whatever
      object_count_changed = true;
      // if object was removed, we need to close its window as well
      if(!ndfbin.contains_object(object_name)) {
        open_object_windows.erase(object_name);
      }
      m_is_changed = true;
    }
  }

  return window_opened;
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

bool wgrd_files::NdfBin::load_xml(fs::path path) {
  if(fs::exists(xml_path)) {
    ndfbin.load_from_xml_file(xml_path);
    return true;
  }
  spdlog::info("No ndf xml file found at {}", xml_path.string());
  return false;
}

bool wgrd_files::NdfBin::save_xml(fs::path path) {
  ndfbin.save_ndf_xml_to_file(path);
  return true;
}

bool wgrd_files::NdfBin::save_bin(fs::path path) {
  ndfbin.save_ndfbin_to_file(path);
  return true;
}

