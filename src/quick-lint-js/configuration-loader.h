// Copyright (C) 2020  Matthew "strager" Glazar
// See end of file for extended copyright information.

#ifndef QUICK_LINT_JS_CONFIGURATION_LOADER_H
#define QUICK_LINT_JS_CONFIGURATION_LOADER_H

#include <optional>
#include <quick-lint-js/configuration.h>
#include <quick-lint-js/file-canonical.h>
#include <quick-lint-js/file.h>
#include <quick-lint-js/result.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace quick_lint_js {
struct file_to_lint;

class configuration_filesystem {
 public:
  virtual ~configuration_filesystem() = default;

  virtual result<canonical_path_result, canonicalize_path_io_error>
  canonicalize_path(const std::string&) = 0;
  virtual result<padded_string, read_file_io_error, platform_file_io_error>
  read_file(const canonical_path&) = 0;
};

struct configuration_change {
  const std::string* watched_path;  // Never nullptr.
  configuration* config;            // Never nullptr.

  // token is the pointer given to
  // configuration_loader::watch_and_load_for_file.
  void* token;
};

class configuration_loader {
 public:
  explicit configuration_loader(configuration_filesystem*);

  configuration_filesystem* fs() noexcept { return this->fs_; }

  result<configuration*, canonicalize_path_io_error, read_file_io_error,
         platform_file_io_error>
  watch_and_load_for_file(const std::string& file_path, const void* token);
  result<configuration*, canonicalize_path_io_error, read_file_io_error,
         platform_file_io_error>
  load_for_file_2(const std::string& file_path);
  result<configuration*, canonicalize_path_io_error, read_file_io_error,
         platform_file_io_error>
  load_for_file_2(const file_to_lint&);

  std::vector<configuration_change> refresh();

 private:
  struct loaded_config_file {
    configuration config;
    padded_string file_content;
  };

  struct found_config_file {
    std::optional<canonical_path> path;
    loaded_config_file* already_loaded = nullptr;
    padded_string file_content{};
  };

  struct watched_path {
    std::string input_path;
    std::optional<canonical_path> config_path;
    std::optional<std::variant<canonicalize_path_io_error, read_file_io_error,
                               platform_file_io_error>>
        error;
    void* token;
  };

  result<configuration*, canonicalize_path_io_error, read_file_io_error,
         platform_file_io_error>
  load_config_file(const char* config_path);
  result<configuration*, canonicalize_path_io_error, read_file_io_error,
         platform_file_io_error>
  find_and_load_config_file_for_input(const char* input_path);
  result<configuration*, canonicalize_path_io_error, read_file_io_error,
         platform_file_io_error>
  find_and_load_config_file_for_current_directory();

  result<configuration*, read_file_io_error, platform_file_io_error>
  find_and_load_config_file_in_directory_and_ancestors(canonical_path&&,
                                                       const char* input_path);
  result<found_config_file, read_file_io_error, platform_file_io_error>
  find_config_file_in_directory_and_ancestors(canonical_path&&);

  result<canonical_path_result, canonicalize_path_io_error>
  get_parent_directory(const char* input_path);

  loaded_config_file* get_loaded_config(const canonical_path& path) noexcept;

  configuration_filesystem* fs_;
  configuration default_config_;

  // Key: config file path
  // Value: cached parsed configuration
  std::unordered_map<canonical_path, loaded_config_file> loaded_config_files_;

  std::vector<watched_path> watched_paths_;
};

class basic_configuration_filesystem : public configuration_filesystem {
 public:
  static basic_configuration_filesystem* instance() noexcept;

  result<canonical_path_result, canonicalize_path_io_error> canonicalize_path(
      const std::string&) override;
  result<padded_string, read_file_io_error, platform_file_io_error> read_file(
      const canonical_path&) override;
};
}

#endif

// quick-lint-js finds bugs in JavaScript programs.
// Copyright (C) 2020  Matthew "strager" Glazar
//
// This file is part of quick-lint-js.
//
// quick-lint-js is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// quick-lint-js is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with quick-lint-js.  If not, see <https://www.gnu.org/licenses/>.
