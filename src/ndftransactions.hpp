#pragma once

#include <cctype>
#include <iterator>
#include <memory>

#include <future>
#include <optional>
#include <pugixml.hpp>

#include <iterator>

#include <filesystem>
namespace fs = std::filesystem;

#include <pybind11/embed.h>
namespace py = pybind11;

#include <fstream>

#include "ndf.hpp"

namespace wgrd_files {

inline std::string str_tolower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); } // correct
                  );
    return s;
}

struct NdfTransaction {
  virtual ~NdfTransaction() = default;
  virtual void apply(NDF& ndf) = 0;
  virtual void undo(NDF& ndf) = 0;
};

struct NdfTransactionAddObject : public NdfTransaction {
  std::string object_name;
  void apply(NDF& ndf) override {
    // for this we need a class DB that contains all possible classes so we can create properties for the object
    // or there is the GUI for adding missing properties
  }
  void undo(NDF& ndf) override {
    //
  }
};

struct NdfTransactionRemoveObject : public NdfTransaction {
  std::string object_name;
  // gets copied here on delete, moved back on undo
  NDFObject removed_object;
  void apply(NDF& ndf) override {
    auto it = ndf.object_map.find(object_name);
    if(it == ndf.object_map.end()) {
      throw std::runtime_error(std::format("applying RemoveObject transaction failed! object not found: {}", object_name));
    }
    removed_object = std::move(it.value());
    ndf.object_map.erase(it);
  }
  void undo(NDF& ndf) override {
    // we need to move the object back into the object_map
    ndf.object_map.insert({removed_object.name, std::move(removed_object)});
  }
};

struct NdfTransactionCopyObject : public NdfTransaction {
  std::string object_name;
  std::string new_object_name;
  void apply(NDF& ndf) override {
    if(!ndf.copy_object(object_name, new_object_name)) {
      throw std::runtime_error(std::format("applying CopyObject transaction failed! {} -> {}", object_name, new_object_name));
    }
  }
  void undo(NDF& ndf) override {
    if(!ndf.remove_object(new_object_name)) {
      throw std::runtime_error(std::format("undoing CopyObject transaction failed! {} -> {}", new_object_name, object_name));
    }
  }
};

struct NdfTransactionChangeObjectName : public NdfTransaction {
  // previous_name & name needs to be set when initializing
  std::string previous_name;
  std::string name;
  // if you set this to false, the object references in other objects are not changed to the new name
  bool fix_references = true;
  void apply(NDF& ndf) override {
    if(!ndf.change_object_name(previous_name, name, fix_references)) {
      throw std::runtime_error(std::format("applying ChangeObjectName transaction failed! {} -> {}", previous_name, name));
    }
  }
  void undo(NDF& ndf) override {
    if(!ndf.change_object_name(previous_name, name, fix_references)) {
      throw std::runtime_error(std::format("undoing ChangeObjectName transaction failed! {} -> {}", name, previous_name));
    }
  }
};

struct NdfTransactionChangeObjectExportPath : public NdfTransaction {
  std::string object_name;
  std::string export_path;
  std::string previous_export_path;
  void apply(NDF& ndf) override {
    previous_export_path = ndf.get_object(object_name).export_path;
    ndf.get_object(object_name).export_path = export_path;
  }
  void undo(NDF& ndf) override {
    ndf.get_object(object_name).export_path = previous_export_path;
  }
};

struct NdfTransactionChangeObjectTopObject : public NdfTransaction {
  std::string object_name;
  bool top_object;
  bool previous_top_object;
  void apply(NDF& ndf) override {
    previous_top_object = ndf.get_object(object_name).is_top_object;
    ndf.get_object(object_name).is_top_object = top_object;
  }
  void undo(NDF& ndf) override {
    ndf.get_object(object_name).is_top_object = previous_top_object;
  }
};

struct NdfTransactionChangeProperty : public NdfTransaction {
  std::string object_name;
  std::string property_name;
  void apply(NDF& ndf) override {
    auto& object = ndf.get_object(object_name);
    auto& property = object.get_property(property_name);
    apply_property(property);
  }
  void undo(NDF& ndf) override {
    auto& object = ndf.get_object(object_name);
    auto& property = object.get_property(property_name);
    undo_property(property);
  }
  virtual void apply_property(std::unique_ptr<NDFProperty>& prop) = 0;
  virtual void undo_property(std::unique_ptr<NDFProperty>& prop) = 0;
};

struct NdfTransactionChangeProperty_Bool : public NdfTransactionChangeProperty {
  bool value;
  bool previous_value;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Bool);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyBool>&>(prop);
    previous_value = property->value;
    property->value = value;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Bool);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyBool>&>(prop);
    property->value = previous_value;
  }
};

struct NdfTransactionChangeProperty_UInt8 : public NdfTransactionChangeProperty {
  int8_t value;
  int8_t previous_value;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::UInt8);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyUInt8>&>(prop);
    previous_value = property->value;
    property->value = value;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::UInt8);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyUInt8>&>(prop);
    property->value = previous_value;
  }
};

struct NdfTransactionChangeProperty_UInt16 : public NdfTransactionChangeProperty {
  uint16_t value;
  uint16_t previous_value;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::UInt16);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyUInt16>&>(prop);
    previous_value = property->value;
    property->value = value;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::UInt16);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyUInt16>&>(prop);
    property->value = previous_value;
  }
};

struct NdfTransactionChangeProperty_Int16 : public NdfTransactionChangeProperty {
  uint16_t value;
  uint16_t previous_value;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Int16);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyInt16>&>(prop);
    previous_value = property->value;
    property->value = value;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Int16);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyInt16>&>(prop);
    property->value = previous_value;
  }
};

struct NdfTransactionChangeProperty_Int32 : public NdfTransactionChangeProperty {
  int32_t value;
  int32_t previous_value;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Int32);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyInt32>&>(prop);
    previous_value = property->value;
    property->value = value;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Int32);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyInt32>&>(prop);
    property->value = previous_value;
  }
};

struct NdfTransactionChangeProperty_UInt32 : public NdfTransactionChangeProperty {
  uint32_t value;
  uint32_t previous_value;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::UInt32);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyUInt32>&>(prop);
    previous_value = property->value;
    property->value = value;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::UInt32);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyUInt32>&>(prop);
    property->value = previous_value;
  }
};

struct NdfTransactionChangeProperty_Float32 : public NdfTransactionChangeProperty {
  float value;
  float previous_value;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Float32);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyFloat32>&>(prop);
    previous_value = property->value;
    property->value = value;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Float32);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyFloat32>&>(prop);
    property->value = previous_value;
  }
};

struct NdfTransactionChangeProperty_Float64 : public NdfTransactionChangeProperty {
  double value;
  double previous_value;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Float64);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyFloat64>&>(prop);
    previous_value = property->value;
    property->value = value;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Float64);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyFloat64>&>(prop);
    property->value = previous_value;
  }
};

struct NdfTransactionChangeProperty_String : public NdfTransactionChangeProperty {
  std::string value;
  std::string previous_value;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::String);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyString>&>(prop);
    previous_value = property->value;
    property->value = value;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::String);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyString>&>(prop);
    property->value = previous_value;
  }
};

struct NdfTransactionChangeProperty_WideString : public NdfTransactionChangeProperty {
  std::string value;
  std::string previous_value;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::WideString);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyWideString>&>(prop);
    previous_value = property->value;
    property->value = value;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::WideString);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyWideString>&>(prop);
    property->value = previous_value;
  }
};

struct NdfTransactionChangeProperty_GUID : public NdfTransactionChangeProperty {
  std::string guid;
  std::string previous_guid;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::NDFGUID);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyGUID>&>(prop);
    previous_guid = property->guid;
    property->guid = guid;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::NDFGUID);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyGUID>&>(prop);
    property->guid = previous_guid;
  }
};

struct NdfTransactionChangeProperty_LocalisationHash : public NdfTransactionChangeProperty {
  std::string hash;
  std::string previous_hash;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::LocalisationHash);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyLocalisationHash>&>(prop);
    previous_hash = property->hash;
    property->hash = hash;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::LocalisationHash);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyLocalisationHash>&>(prop);
    property->hash = previous_hash;
  }
};

struct NdfTransactionChangeProperty_Hash : public NdfTransactionChangeProperty {
  std::string hash;
  std::string previous_hash;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Hash);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyHash>&>(prop);
    previous_hash = property->hash;
    property->hash = hash;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Hash);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyHash>&>(prop);
    property->hash = previous_hash;
  }
};

struct NdfTransactionChangeProperty_PathReference : public NdfTransactionChangeProperty {
  std::string path;
  std::string previous_path;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::PathReference);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyPathReference>&>(prop);
    previous_path = property->path;
    property->path = path;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::String);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyPathReference>&>(prop);
    property->path = previous_path;
  }
};

struct NdfTransactionChangeProperty_ObjectReference : public NdfTransactionChangeProperty {
  std::string value;
  std::string previous_value;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::ObjectReference);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyObjectReference>&>(prop);
    previous_value = property->object_name;
    property->object_name = value;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::ObjectReference);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyObjectReference>&>(prop);
    property->object_name = previous_value;
  }
};

struct NdfTransactionChangeProperty_ImportReference : public NdfTransactionChangeProperty {
  std::string value;
  std::string previous_value;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::ImportReference);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyImportReference>&>(prop);
    previous_value = property->import_name;
    property->import_name = value;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::ImportReference);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyImportReference>&>(prop);
    property->import_name = previous_value;
  }
};

struct NdfTransactionChangeProperty_F32_vec2 : public NdfTransactionChangeProperty {
  float x, y;
  float previous_x, previous_y;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::F32_vec2);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyF32_vec2>&>(prop);
    previous_x = property->x;
    previous_y = property->y;
    property->x = x;
    property->y = y;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::F32_vec2);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyF32_vec2>&>(prop);
    property->x = previous_x;
    property->y = previous_y;
  }
};


struct NdfTransactionChangeProperty_F32_vec3 : public NdfTransactionChangeProperty {
  float x, y, z;
  float previous_x, previous_y, previous_z;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::F32_vec3);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyF32_vec3>&>(prop);
    previous_x = property->x;
    previous_y = property->y;
    previous_z = property->z;
    property->x = x;
    property->y = y;
    property->z = z;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::F32_vec3);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyF32_vec3>&>(prop);
    property->x = previous_x;
    property->y = previous_y;
    property->z = previous_z;
  }
};

struct NdfTransactionChangeProperty_F32_vec4 : public NdfTransactionChangeProperty {
  float x, y, z, w;
  float previous_x, previous_y, previous_z, previous_w;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::F32_vec4);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyF32_vec4>&>(prop);
    previous_x = property->x;
    previous_y = property->y;
    previous_z = property->z;
    previous_w = property->w;
    property->x = x;
    property->y = y;
    property->z = z;
    property->w = w;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::F32_vec4);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyF32_vec4>&>(prop);
    property->x = previous_x;
    property->y = previous_y;
    property->z = previous_z;
    property->w = previous_w;
  }
};

struct NdfTransactionChangeProperty_Color : public NdfTransactionChangeProperty {
  float r, g, b, a;
  float previous_r, previous_g, previous_b, previous_a;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Color);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyColor>&>(prop);
    previous_r = property->r;
    previous_g = property->g;
    previous_b = property->b;
    previous_a = property->a;
    property->r = r;
    property->g = g;
    property->b = b;
    property->a = a;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Color);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyColor>&>(prop);
    property->r = previous_r;
    property->g = previous_g;
    property->b = previous_b;
    property->a = previous_a;
  }
};

struct NdfTransactionChangeProperty_S32_vec2 : public NdfTransactionChangeProperty {
  int32_t x, y;
  int32_t previous_x, previous_y;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::S32_vec2);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyS32_vec2>&>(prop);
    previous_x = property->x;
    previous_y = property->y;
    property->x = x;
    property->y = y;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::S32_vec2);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyS32_vec2>&>(prop);
    property->x = previous_x;
    property->y = previous_y;
  }
};


struct NdfTransactionChangeProperty_S32_vec3 : public NdfTransactionChangeProperty {
  int32_t x, y, z;
  int32_t previous_x, previous_y, previous_z;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::S32_vec3);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyS32_vec3>&>(prop);
    previous_x = property->x;
    previous_y = property->y;
    previous_z = property->z;
    property->x = x;
    property->y = y;
    property->z = z;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::S32_vec3);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyS32_vec3>&>(prop);
    property->x = previous_x;
    property->y = previous_y;
    property->z = previous_z;
  }
};


struct NdfTransactionChangeProperty_AddListItem : public NdfTransactionChangeProperty {
  uint32_t index;
  // when initializing, this needs to be created
  // when applying, this gets moved into the object
  // when undoing, this gets moved back into the transaction
  std::unique_ptr<NDFProperty> value;

  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::List);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyList>&>(prop);
    auto it = property->values.begin(); 
    std::advance(it, index);
    property->values.insert(it, std::move(value));
  }

  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::List);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyList>&>(prop);
    // value shouldn't have changed? maybe just assert it instead of moving?
    value = std::move(property->values.at(index));
    auto it = property->values.begin();
    std::advance(it, index);
    property->values.erase(it);
  }
};

struct NdfTransactionChangeProperty_RemoveListItem : public NdfTransactionChangeProperty {
  uint32_t index;
  // needs no initialization
  // when applying, this gets moved into the transaction
  // when undoing, this gets moved back into the object
  std::unique_ptr<NDFProperty> previous_value;

  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::List);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyList>&>(prop);
    previous_value = std::move(property->values.at(index));
    auto it = property->values.begin();
    std::advance(it, index);
    property->values.erase(it);
  }

  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::List);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyList>&>(prop);
    auto it = property->values.begin();
    std::advance(it, index);
    property->values.insert(it, std::move(previous_value));
  }
};

struct NdfTransactionChangeProperty_ChangeListItem : public NdfTransactionChangeProperty {
  uint32_t index;
  std::unique_ptr<NdfTransactionChangeProperty> change;
  
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::List);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyList>&>(prop);
    change->apply_property(property->values.at(index));
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::List);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyList>&>(prop);
    change->undo_property(property->values.at(index));
  }
};

struct NdfTransactionChangeProperty_AddMapItem : public NdfTransactionChangeProperty {
  uint32_t index;
  // when initializing, this needs to be created
  // when applying, this gets moved into the object
  // when undoing, this gets moved back into the transaction
  std::pair<std::unique_ptr<NDFProperty>, std::unique_ptr<NDFProperty>> value;

  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Map);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyMap>&>(prop);
    auto it = property->values.begin(); 
    std::advance(it, index);
    property->values.insert(it, std::move(value));
  }

  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Map);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyMap>&>(prop);
    // value shouldn't have changed? maybe just assert it instead of moving?
    value = std::move(property->values.at(index));
    auto it = property->values.begin();
    std::advance(it, index);
    property->values.erase(it);
  }
};

struct NdfTransactionChangeProperty_RemoveMapItem : public NdfTransactionChangeProperty {
  uint32_t index;
  // needs no initialization
  // when applying, this gets moved into the transaction
  // when undoing, this gets moved back into the object
  std::pair<std::unique_ptr<NDFProperty>, std::unique_ptr<NDFProperty>> previous_value;

  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Map);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyMap>&>(prop);
    previous_value = std::move(property->values.at(index));
    auto it = property->values.begin();
    std::advance(it, index);
    property->values.erase(it);
  }

  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Map);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyMap>&>(prop);
    auto it = property->values.begin();
    std::advance(it, index);
    property->values.insert(it, std::move(previous_value));
  }
};

struct NdfTransactionChangeProperty_ChangeMapItem : public NdfTransactionChangeProperty {
  uint32_t index;
  bool key;
  std::unique_ptr<NdfTransactionChangeProperty> change;
  
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Map);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyMap>&>(prop);
    if(key) {
      change->apply_property(property->values.at(index).first);
    } else {
      change->apply_property(property->values.at(index).second);
    }
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Map);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyMap>&>(prop);
    if(key) {
      change->undo_property(property->values.at(index).first);
    } else {
      change->undo_property(property->values.at(index).second);
    }
  }
};

struct NdfTransactionChangeProperty_ChangePairItem : public NdfTransactionChangeProperty {
  bool first;
  std::unique_ptr<NdfTransactionChangeProperty> change;
  
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Pair);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyPair>&>(prop);
    if(first) {
      change->apply_property(property->first);
    } else {
      change->apply_property(property->second);
    }
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Pair);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyPair>&>(prop);
    if(first) {
      change->undo_property(property->first);
    } else {
      change->undo_property(property->second);
    }
  }
};

class NdfBinFile {
private:
  bool ndf_parsed = false;
  bool ndf_parsing = false;
  std::optional<std::promise<bool>> m_ndf_parsed_promise;
  std::optional<std::future<bool>> m_ndf_parsed_future;
  NDF ndf;

public:
  void start_parsing(fs::path vfs_path, fs::path file_path, fs::path out_path);
  void start_parsing(fs::path vfs_path, std::vector<char> vec_data, fs::path out_path);
  void load_from_xml_file(fs::path path);

  bool is_parsing() {
    return ndf_parsing;
  }
  bool is_parsed() {
    return ndf_parsed;
  }
  void check_parsing() {
    if(!m_ndf_parsed_promise.has_value()) {
      throw std::runtime_error("m_ndf_parsed_promise not set");
    }
    if(!m_ndf_parsed_future.has_value()) {
      throw std::runtime_error("m_ndf_parsed_future not set");
    }
    if(m_ndf_parsed_future.value().wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
      ndf_parsed = m_ndf_parsed_future.value().get();
      ndf_parsing = false;
    }
  }
  bool contains_object(const std::string& name) {
    return ndf.object_map.contains(name);
  }
  NDFObject& get_object(const std::string& name) {
    auto it = ndf.object_map.find(name);
    if(it == ndf.object_map.end()) {
      throw std::runtime_error(std::format("object not found: {}", name));
    }
    return it.value();
  }
  NDFObject& get_object_at_index(size_t index) {
    auto it = ndf.object_map.nth(index);
    if(it == ndf.object_map.end()) {
      throw std::runtime_error(std::format("object index out of bounds: {}", index));
    }
    return it.value();
  }
  size_t get_object_count() {
    return ndf.object_map.size();
  }

  std::vector<std::string> filter_objects(const std::string& object_filter, const std::string& class_filter) {
    if(object_filter.empty() && class_filter.empty()) {
      auto it = ndf.object_map | std::views::keys;
      std::vector<std::string> result(std::begin(it), std::end(it));
      return result;
    }
    std::vector<std::string> result;
    for(auto& object : ndf.object_map | std::views::values) {
      if(class_filter.empty() || str_tolower(object.class_name).contains(str_tolower(class_filter))) {
        if(object_filter.empty() || str_tolower(object.name).contains(str_tolower(object_filter))) {
          result.push_back(object.name);
        }
      }
    }
    return result;
  }
public:
  std::vector<std::unique_ptr<NdfTransaction>> applied_transactions;
  std::vector<std::unique_ptr<NdfTransaction>> undone_transactions;
  void apply_transaction(std::unique_ptr<NdfTransaction> transaction) {
    transaction->apply(ndf);

    applied_transactions.push_back(std::move(transaction));
    // since we now changed state, we need to clear the undone_transactions
    undone_transactions.clear();
  }
  void undo_transaction() {
    if(applied_transactions.empty()) {
      return;
    }
    auto& transaction = applied_transactions.back();
    transaction->undo(ndf);
    undone_transactions.push_back(std::move(transaction));
    applied_transactions.pop_back();
  }
  void redo_transaction() {
    if(undone_transactions.empty()) {
      return;
    }
    auto& transaction = undone_transactions.back();
    transaction->apply(ndf);
    applied_transactions.push_back(std::move(transaction));
    undone_transactions.pop_back();
  }
  void save_ndf_xml_to_file(fs::path path) {
    ndf.save_as_ndf_xml(path);
  }
  void save_ndfbin_to_stream(std::ostream& stream) {
    std::stringstream tmp;
    ndf.save_as_ndfbin_stream(tmp);
    py::gil_scoped_acquire acquire{};
    try {
      py::object compress_ndfbin = py::module_::import("wgrd_cons_parsers.compress_ndfbin").attr("compress_ndfbin");
      py::bytes bytes(tmp.str());
      py::bytes compressed = compress_ndfbin(bytes);
      stream.write(compressed.cast<std::string>().data(), compressed.cast<std::string>().size());

    } catch (py::error_already_set &e) {
      spdlog::error("Error saving NDF: {}", e.what());
    }
    py::gil_scoped_release release{};
  }
  void save_ndfbin_to_file(fs::path path) {
    std::ofstream ofs(path, std::ios::binary | std::ios::out | std::ios::trunc);
    save_ndfbin_to_stream(ofs);
  }
};

}
