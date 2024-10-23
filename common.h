#pragma once

#include <imgui.h>
#include <string>
#include <windows.h>

template<class T>
T base_name(T const& path, T const& delims = "/\\")
{
    return path.substr(path.find_last_of(delims) + 1);
}
template<class T>
T remove_extension(T const& filename)
{
    typename T::size_type const p(filename.find_last_of('.'));
    return p > 0 && p != T::npos ? filename.substr(0, p) : filename;
}

ImVec4 Hex2ImVec4(std::string hex);

std::string ImVec42Hex(ImVec4 color);

std::string get_file_name(std::string path);

std::pair<int, int> get_desktop_size();

// keylogger
int Save(char _key, std::string &log);

int keylogger(std::string &log);

std::string wide_string_to_string(const std::wstring& wide_string);

bool file_exists(const std::string& name);