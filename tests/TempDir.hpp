// tests/TempDir.hpp
// A unique scratch directory under the system temp folder, deleted when the test ends.
#pragma once
#include <filesystem>
#include <fstream>
#include <random>
#include <string>

class TempDir {
private:
    std::filesystem::path m_path;

public:
    TempDir() {
        std::random_device rd;
        m_path = std::filesystem::temp_directory_path() / ("tiluv-test-" + std::to_string(rd()));
        std::filesystem::create_directories(m_path);
    }
    ~TempDir() {
        std::error_code ec;
        std::filesystem::remove_all(m_path, ec);
    }
    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

    const std::filesystem::path& path() const { return m_path; }

    // Creates `relative` (and its parent folders) with the given content; returns the full path.
    std::string touch(const std::string& relative, const std::string& content = "") const {
        const std::filesystem::path file = m_path / relative;
        std::filesystem::create_directories(file.parent_path());
        std::ofstream(file) << content;
        return file.string();
    }
};
