#pragma once

#include "files/files.hpp"

namespace wgrd_files {

class File {
private:
  std::atomic_bool m_is_parsed = false;
  std::atomic_bool m_is_parsing = false;

  std::optional<std::promise<bool>> m_parsed_promise;
  std::optional<std::future<bool>> m_parsed_future;

protected:
  const wgrd_files::Files *files;
  // stores the actual informations regarding the dat file
  FileMeta meta;
  // path to the database folder, dbs are shared between all files
  fs::path db_path;
  // path to the tmp directory of this file
  // (may be used while parsing or saving, will be deleted afterwards)
  fs::path tmp_path;
  // path to the binary output file of this file
  fs::path bin_path;
  // path to the xml file describing this file
  // (may not exist, will be created if it doesn't exist yet on parsing or
  // save_xml call)
  fs::path xml_path;
  // FIXME: maybe we should just create a transaction list here instead
  // of having different transaction lists that mark this flag themselves
  bool m_is_changed = false;

  friend class Files;

public:
  explicit File(const Files *files, FileMeta meta);
  virtual FileType get_type() { return FileType::UNKNOWN; }
  void render();
  // this function should call the imgui shortcuts
  virtual void render_shortcuts();
  // this function is called *inside* the BeginMenuBar call
  virtual void render_menu();
  // this function is called *inside* the window
  virtual void render_window();
  // this function is called *outside* the window (e.g. ndfbin uses this to
  // spawn new object windows)
  virtual void render_extra() {};
  // this function returns the bytes stored in the given filestream
  std::vector<char> get_data();
  // this function just plainly copies from the given filestream to the given
  // path
  bool copy_to_file(fs::path path);

  void start_parsing(bool try_xml = true);

  // default implementation, may be overridden
  virtual bool load_stream() {
    copy_to_file(bin_path);
    return load_bin(bin_path);
  }

  virtual bool load_xml(fs::path path) {
    spdlog::error("NOT IMPLEMENTED cannot load xml file {} into {}",
                  meta.vfs_path, path.string());
    return false;
  }
  virtual bool save_xml(fs::path path) {
    spdlog::error("NOT IMPLEMENTED cannot save xml file {} into {}",
                  meta.vfs_path, path.string());
    return false;
  }
  virtual bool load_bin(fs::path path) {
    spdlog::error("NOT IMPLEMENTED cannot save bin file {} into {}",
                  meta.vfs_path, path.string());
    return false;
  }
  virtual bool save_bin(fs::path path) {
    spdlog::error("NOT IMPLEMENTED cannot save bin file {} into {}",
                  meta.vfs_path, path.string());
    return false;
  }

  virtual bool undo() {
    spdlog::error("NOT IMPLEMENTED cannot undo {}", meta.vfs_path);
    return false;
  }
  virtual bool redo() {
    spdlog::error("NOT IMPLEMENTED cannot redo {}", meta.vfs_path);
    return false;
  }

  virtual bool is_changed() { return m_is_changed; }

  // virtual since edat overwrites it to pass down to workspace
  virtual void check_parsing() {
    if (m_is_parsed) {
      return;
    }
    if (!m_is_parsing) {
      return;
    }
    if (!m_parsed_promise.has_value()) {
      throw std::runtime_error("m_ndf_parsed_promise not set");
    }
    if (!m_parsed_future.has_value()) {
      throw std::runtime_error("m_ndf_parsed_future not set");
    }
    if (m_parsed_future.value().wait_for(std::chrono::seconds(0)) ==
        std::future_status::ready) {
      m_is_parsed = m_parsed_future.value().get();
      m_is_parsing = false;
    }
  }

  // virtual since edat overwrites it to pass down to workspace
  virtual bool is_parsed() { return m_is_parsed; }

  // virtual since edat overwrites it to pass down to workspace
  virtual bool is_parsing() { return m_is_parsing; }
};

typedef std::vector<std::unique_ptr<File>> FileList;

} // namespace wgrd_files
