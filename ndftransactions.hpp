#pragma once

#include <memory>

#include <future>
#include <optional>
#include <pugixml.hpp>

#include <filesystem>
namespace fs = std::filesystem;

#include <pybind11/embed.h>
namespace py = pybind11;

#include <fstream>

#include "ndf.hpp"

namespace wgrd_files {

struct NdfTransaction {
  virtual ~NdfTransaction() = default;
  virtual void apply(NDF& ndf) = 0;
  virtual void undo(NDF& ndf) = 0;
};

struct NdfTransactionAddObject : public NdfTransaction {
  std::string object_name;
  void apply(NDF& ndf) override {
    //
  }
  void undo(NDF& ndf) override {
    //
  }
};

struct NdfTransactionRemoveObject : public NdfTransaction {
  std::string object_name;
  void apply(NDF& ndf) override {
    //
  }
  void undo(NDF& ndf) override {
    //
  }
};

struct NdfTransactionChangeProperty : public NdfTransaction {
  std::string object_name;
  std::string property_name;
  void apply(NDF& ndf) override {
    auto& object = ndf.get_obj_ref(object_name);
    auto& property = object.get_property(property_name);
    apply_property(property);
  }
  void undo(NDF& ndf) override {
    auto& object = ndf.get_obj_ref(object_name);
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

struct NdfTransactionChangeProperty_Int8 : public NdfTransactionChangeProperty {
  int8_t value;
  int8_t previous_value;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Int8);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyInt8>&>(prop);
    previous_value = property->value;
    property->value = value;
  }
  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::Int8);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyInt8>&>(prop);
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

struct NdfTransactoinChangeProperty_Float32 : public NdfTransactionChangeProperty {
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

struct NdfTransactionChangeProperty_AddListItem : public NdfTransactionChangeProperty {
  // when initializing, this needs to be created
  // when applying, this gets moved into the object
  // when undoing, this gets moved back into the transaction
  std::unique_ptr<NDFProperty> value;

  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::List);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyList>&>(prop);
    property->values.push_back(std::move(value));
  }

  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::List);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyList>&>(prop);
    value = std::move(property->values.back());
    property->values.pop_back();
  }
};

struct NdfTransactionChangeProperty_RemoveListItem : public NdfTransactionChangeProperty {
  // needs no initialization
  // when applying, this gets moved into the transaction
  // when undoing, this gets moved back into the object
  std::unique_ptr<NDFProperty> previous_value;
  void apply_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::List);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyList>&>(prop);
    previous_value = std::move(property->values.back());
    property->values.pop_back();
  }

  void undo_property(std::unique_ptr<NDFProperty>& prop) override {
    assert(prop->property_type == NDFPropertyType::List);
    auto& property = reinterpret_cast<std::unique_ptr<NDFPropertyList>&>(prop);
    property->values.push_back(std::move(previous_value));
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

class NdfBinFile {
private:
  bool ndf_parsed = false;
  bool ndf_parsing = false;
  std::optional<std::promise<bool>> m_ndf_parsed_promise;
  std::optional<std::future<bool>> m_ndf_parsed_future;
  NDF ndf;

public:
  void start_parsing(fs::path path);
  bool load_from_file(fs::path path);

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
  size_t get_object_count() {
    return ndf.objects.size();
  }

  NDFObject& get_object(size_t index) {
    return ndf.objects.at(index);
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
};

}
