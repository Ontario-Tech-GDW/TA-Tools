#include "zipping.h"
#include "minizip/mz_compat.h"
#include <filesystem>
#include <ctime>


uint32_t filetime(const char* f, tm_zip* tmzip, uint32_t* dt)
{
	int ret = 0;
	{
		/*FILETIME ftLocal;
		HANDLE hFind;
		WIN32_FIND_DATAA ff32;

		hFind = FindFirstFileA(f, &ff32);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			FileTimeToLocalFileTime(&(ff32.ftLastWriteTime), &ftLocal);
			FileTimeToDosDateTime(&ftLocal, ((LPWORD)dt) + 1, ((LPWORD)dt) + 0);
			FindClose(hFind);
			ret = 1;
		}*/
	}
	return ret;
}


ZipFile::ZipFile(const char* filename) {
	file_ = zipOpen64(filename, 0);
	if (file_ == nullptr) {
		throw std::exception("Failed to open output file");
	}
	else {
		fprintf(stdout, "Opening zip file: \"%s\"\r\n", filename);
	}

	rootDir_ = std::filesystem::current_path();
}

ZipFile::~ZipFile() {
	int err = zipClose(file_, nullptr);
	if (err != ZIP_OK) {
		fprintf(stderr, "Failed to close zip file");
	}
}

bool ZipFile::AddFile(const char* filename)
{
	const size_t BUFFER_SIZE = 8192;

	if (filename == nullptr) {
		fprintf(stderr, "Filename is null, skipping");
		return false;
	}

	if (!std::filesystem::exists(filename)) {
		fprintf(stderr, "Filename \"%s\" does not exist, skipping", filename);
		return false;
	}

	if (file_ != nullptr) {
		FILE* file = nullptr;
		uint8_t buffer[BUFFER_SIZE];
		size_t fileSize = 0;

		fopen_s(&file, filename, "r");
		if (file == nullptr) {
			fprintf(stderr, "Failed to open file \"%s\" for reading, skipping", filename);
			return false;
		}

		// Extract file size
		fseek(file, 0, SEEK_END);
		fileSize = ftell(file);
		rewind(file);

		int is64bit = fileSize > 0xffffffff ? 1 : 0;

		// Configure file info to be zipped
		zip_fileinfo info;
		info.tmz_date.tm_sec = info.tmz_date.tm_min = info.tmz_date.tm_hour =
			info.tmz_date.tm_mday = info.tmz_date.tm_mon = info.tmz_date.tm_year = 0;
		info.dosDate = 0;
		info.internal_fa = 0;
		info.external_fa = 0;

		// Get the file time for the zip
		filetime(filename, &info.tmz_date, &info.dosDate);

		std::string relative = std::filesystem::relative(filename, rootDir_).string();

		// Add file to zip
		int result = zipOpenNewFileInZip_64(file_, relative.c_str(), &info,
			NULL, 0,
			NULL, 0, NULL,
			Z_DEFLATED, 6, is64bit);

		// On failure, close and return false
		if (result != ZIP_OK) {
			fprintf(stderr, "Failed to add file to zip");
			fclose(file);
			return false;
		}

		size_t read = 0;
		while ((read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
			int result = zipWriteInFileInZip(file_, buffer, read);
			if (result != ZIP_OK) {
				fclose(file);
				zipCloseFileInZip(file_);
				return false;
			}
		}

		fprintf(stdout, "Added \"%s\" to zip file\r\n", filename);
		fclose(file);
		zipCloseFileInZip(file_);
		return true;
	}
	else {
		fprintf(stderr, "Zip file not open");
		return false;
	}
}

bool ZipFile::AddDirectory(const char* path)
{
	if (!std::filesystem::is_directory(path)) {
		fprintf(stderr, "\"%s\" is not a directory", path);
	}

	std::string relative = std::filesystem::relative(path, rootDir_).string();

	std::string temp = relative;
	if (temp[temp.size()] != '/') {
		temp += '/';
	}


	// Configure file info to be zipped
	zip_fileinfo info;
	info.tmz_date.tm_sec = info.tmz_date.tm_min = info.tmz_date.tm_hour =
		info.tmz_date.tm_mday = info.tmz_date.tm_mon = info.tmz_date.tm_year = 0;
	info.dosDate = 0;
	info.internal_fa = 0;
	info.external_fa = 0;

	// Get the file time for the zip
	filetime(path, &info.tmz_date, &info.dosDate);

	int result = zipOpenNewFileInZip_64(file_, temp.c_str(), &info, NULL, 0, NULL, 0, NULL, 0, 0, 0);
	if (result != ZIP_OK) {
		fprintf(stderr, "Failed to start directory in zip file");
		return false;
	}
	zipCloseFileInZip(file_);

	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		if (entry.is_directory()) {
			AddDirectory(entry.path().string().c_str());
		}
		else {
			AddFile(entry.path().string().c_str());
		}
	}
}

void ZipFile::SetRoot(const std::filesystem::path& path) {
	rootDir_ = path;
}
