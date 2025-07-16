// test data generator for modding suite,
// creates dat files with some random ndfbin data
// and their corresponding xml files

#include <argparse/argparse.hpp>

#include <experimental/random>
#include <stdint.h>

#include "ndf.hpp"

#include <pybind11/embed.h>
namespace py = pybind11;

#include <filesystem>
namespace fs = std::filesystem;

#include "pugixml.hpp"

void add_random_object(NDF &ndf) {
  NDFObject obj;
  obj.name = "test_object";
  obj.class_name = "TTestClass";
  obj.is_top_object = true;
  obj.export_path = "$/test/object1";

  ndf.add_object(std::move(obj));
}

void add_random_uint8(NDFObject &obj) {
  auto prop = std::make_unique<NDFPropertyUInt8>();
  prop->property_idx = obj.properties.size();
  prop->property_type = NDFPropertyType::UInt8;
  prop->property_name = std::format("TestUInt8_{}", prop->property_idx);
  prop->value = std::experimental::randint(0, 255);
  obj.properties.push_back(std::move(prop));
}

void add_random_uint16(NDFObject &obj) {
  auto prop = std::make_unique<NDFPropertyUInt16>();
  prop->property_idx = obj.properties.size();
  prop->property_type = NDFPropertyType::UInt16;
  prop->property_name = std::format("TestUInt16_{}", prop->property_idx);
  prop->value = std::experimental::randint(0, 65535);
  obj.properties.push_back(std::move(prop));
}

void add_random_uint32(NDFObject &obj) {
  auto prop = std::make_unique<NDFPropertyUInt32>();
  prop->property_idx = obj.properties.size();
  prop->property_type = NDFPropertyType::UInt32;
  prop->property_name = std::format("TestUInt32_{}", prop->property_idx);
  prop->value = std::experimental::randint((int)0, (int)4294967295);
  obj.properties.push_back(std::move(prop));
}

// call with vfs_path -> path to the file
void create_edat(fs::path path,
                 std::unordered_map<std::string, std::string> files) {
  // first we create the xml file for the edat
  pugi::xml_document doc;
  auto root = doc.append_child("EDat");
  root.append_attribute("sectorSize") = 8192;
  root.append_attribute("_wgrd_cons_parsers_version") = "0.2.11";
  for (auto &[vfs_path, _] : files) {
    auto file = root.append_child("File");
    file.append_attribute("path") = vfs_path.c_str();
  }
  fs::path xml_path = path;
  xml_path = xml_path.replace_extension("edat.xml");
  doc.save_file(xml_path.string().c_str());

  // now we create the out folder and copy the files in there
  fs::path out_path = path.parent_path();
  fs::create_directories(out_path / "out");
  for (auto &[vfs_path, fs_path] : files) {
    fs::copy(fs_path, out_path / "out" / vfs_path,
             fs::copy_options::overwrite_existing);
  }

  // now we create the edat file
  py::object edat =
      py::module::import("wgrd_cons_parsers").attr("edat").attr("EdatMain")();

  // as the EdatMain uses the . instead of [] for accessing, we need the
  // dingsda Container
  py::object container =
      py::module::import("dingsda.lib.containers").attr("Container");
  py::dict args = py::dict();
  args["no_alignment"] = true;
  args["disable_checksums"] = true;
  edat.attr("args") = container(args);
  py::object data = edat.attr("get_data")(xml_path.string());
  edat.attr("pack")(xml_path.string(), out_path.string(), data);
}

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program;
  program.add_argument("-o").help(gettext("Path to output folder"));

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error &err) {
    std::cout << err.what() << std::endl;
    std::cout << program;
    exit(0);
  }
  py::scoped_interpreter guard{};

  py::str py_exec = (py::module::import("sys").attr("executable"));
  spdlog::info(std::string(py_exec));
  py::str py_path = (py::module::import("sys").attr("path"));
  spdlog::info(std::string(py_path));

  std::srand(std::time(nullptr));
  fs::path output_folder;
  if (program.present("-o")) {
    output_folder = program.get<std::string>("-o");
  } else {
    output_folder = fs::temp_directory_path() / "modding_suite_test_data";
  }

  fs::create_directories(output_folder / "ndfbin");

  // let's first create some random ndfbin data
  NDF test_ndf;
  {
    add_random_object(test_ndf);
    auto &obj = test_ndf.object_map.begin().value();
    add_random_uint16(obj);
    add_random_uint32(obj);
    add_random_uint8(obj);
    add_random_uint8(obj);
  }
  // now save it in the output folder
  test_ndf.save_as_ndf_xml(output_folder / "ndfbin" / "test.ndfbin.xml");
  test_ndf.save_as_ndfbin(output_folder / "ndfbin" / "test.ndfbin");

  // now generate an edat file containing the ndfbin file
  create_edat(output_folder / "test.dat",
              {{"test", output_folder / "ndfbin" / "test.ndfbin"}});
}
