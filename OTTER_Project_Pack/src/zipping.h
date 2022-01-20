#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <set>
#include <string>

#include "minizip/zip.h"
#include <filesystem>

/**
 * Class for writing to zip files
 */
class ZipFile {
public:
	ZipFile(const char* filename);
	~ZipFile();

	bool AddFile(const char* filename);
	bool AddDirectory(const char* path);

	void SetRoot(const std::filesystem::path& path);

private:
	std::filesystem::path rootDir_;
	zipFile file_;
	std::set<std::string> filenames_;
};