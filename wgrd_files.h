#pragma once

#include <fstream>
#include <pybind11/embed.h>
namespace py = pybind11;

namespace wgrd_files {

class File {
protected:
    std::string vfs_path;
    std::ifstream file;
    size_t offset;
    size_t size;
public:
    explicit File(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
    virtual bool imgui_call();
};

class EDat : public File {
protected:
    py::object vfs_dict;
public:
    explicit EDat(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
    bool imgui_call() override;
    static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class Ess : public File {
public:
    explicit Ess(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
    bool imgui_call() override;
    static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class SFormat : public File {
public:
    explicit SFormat(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
    bool imgui_call() override;
    static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class TGV : public File {
public:
    explicit TGV(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
    bool imgui_call() override;
    static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class PPK : public File {
public:
    explicit PPK(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
    bool imgui_call() override;
    static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class Scenario : public File {
public:
    explicit Scenario(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
    bool imgui_call() override;
    static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class Dic : public File {
public:
    explicit Dic(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
    bool imgui_call() override;
    static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

class NdfBin : public File {
public:
    explicit NdfBin(std::string vfs_path, std::ifstream &f, size_t offset, size_t size);
    bool imgui_call() override;
    static bool is_file(std::string vfs_path, std::ifstream &f, size_t offset);
};

}
