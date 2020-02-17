#ifndef MYFILESYSTEM_HPP
#define MYFILESYSTEM_HPP
#include <vector>
#include <string>

std::vector<std::string> get_files_in_dir(std::string base_dir);

std::string build_path(std::string base_dir, std::string filename);

bool is_dir(std::string path);

#endif