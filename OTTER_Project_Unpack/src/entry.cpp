// OTTER_Project_Unpack.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <shobjidl.h> 
#include <filesystem>
#include <conio.h>

enum ConsoleColor {
	Black     = 0,
	Red       = 1,
	Green     = 2,
	Yellow    = 3,
	Blue      = 4,
	Magenta   = 5,
	Cyan      = 6,
	LightGray = 7
};


struct ConsoleMode {
	ConsoleMode() :
		_foreColor(15),
		_backColor(0),
		_isBold(false),
		_isUnderline(false) {}

	void SetForeColor(ConsoleColor color, bool isBright = true) {
		_foreColor = ((uint8_t)color) + (uint8_t)(isBright ? 90 : 30);
		sprintf_s(buffer, "\x1B[%dm", _foreColor);
		std::cout << buffer;
	}

	void Reset() {
		_foreColor = 15;
		_backColor = 0;
		_isBold    = false;
		_isUnderline = false;
		std::cout << "\x1B[0m";
	}

protected:
	uint8_t _foreColor;
	uint8_t _backColor;
	bool    _isBold;
	bool    _isUnderline;

	char buffer[64];
};

namespace fs = std::filesystem;
int main()
{
	// Allows us to inject ANSII colors into the console
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	unsigned long cMode = 0;
	GetConsoleMode(hConsole, &cMode);
	SetConsoleMode(hConsole, cMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

	ConsoleMode mode;
	mode.SetForeColor(ConsoleColor::Cyan);


	char fileName[4096];

	OPENFILENAMEA fileDialogue ;
	ZeroMemory(&fileDialogue, sizeof(fileDialogue));
	fileDialogue.lStructSize = sizeof (fileDialogue);
	fileDialogue.lpstrTitle = "Select master zip file";
	fileDialogue.hwndOwner = NULL;
	fileDialogue.lpstrFile = fileName;
	fileDialogue.lpstrFile[0] = '\0';
	fileDialogue.nMaxFile = sizeof(fileName);
	fileDialogue.lpstrFilter = "ZIP FILES\0*.ZIP\0";
	fileDialogue.nFilterIndex = 1;
	fileDialogue.lpstrFileTitle = NULL;
	fileDialogue.nMaxFileTitle = 0;
	fileDialogue.lpstrInitialDir=NULL;
	fileDialogue.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	std::cout << "Prompting for master zip path..." << std::endl;
	if (GetOpenFileNameA(&fileDialogue)) {
		fs::path zipPath = fileName;

		fileDialogue.lpstrFilter= "OTTER Root File\0Premake5.lua\0";
		fileDialogue.lpstrTitle = "Select OTTER lua file";

		std::cout << "Prompting OTTER lua file..." << std::endl;
		if (GetOpenFileNameA(&fileDialogue)) {
			fs::path otterPath = fileName;
			otterPath = otterPath.parent_path();

			fs::path tempPath = otterPath / "temp";

			fs::path destination = otterPath / "samples" / zipPath.stem();

			std::cout << "Operating on these folders:" << std::endl;
			std::cout << "Master zip file: " << zipPath.string() << std::endl;
			std::cout << "OTTER root:      " << otterPath.string() << std::endl;
			std::cout << "Projects Target: " << destination.string() << std::endl;

			if (!fs::exists(tempPath)) {
				fs::create_directory(tempPath);
			}
			if (fs::exists(destination)) {
				std::cout << "Found duplicates, delete existing? Y for yes, anything else to abort" << std::endl;
				char c = _getch();
				if (c == 'y' || c == 'Y') {
					std::error_code code;
					fs::remove_all(destination, code);

					if (code.value() != 0) {
						mode.SetForeColor(ConsoleColor::Red);
						std::cout << "Failed to delete duplicate files:" << std::endl;
						std::cout << "\t" << code.message() << std::endl;
						mode.SetForeColor(ConsoleColor::Cyan);
					}
				} else {
					return 1;
				}
			}
			fs::create_directory(destination);

			static char command[4096];
			sprintf_s(command, "tar -xf \"%s\" -C \"%s\"", zipPath.string().c_str(), tempPath.string().c_str());
			std::cout << "Invoking command: " << command << std::endl;
			int retCode = system(command);

			int totalSubmissions = 0;
			int totalInvalid = 0;
			for (const auto& entry : fs::directory_iterator(tempPath)) {
				totalSubmissions++;
				if (entry.path().extension() != ".zip") {
					totalInvalid++;
					mode.SetForeColor(ConsoleColor::Red);
					std::cout << "NOT A ZIP FILE: " << entry.path().string() << std::endl;
					mode.SetForeColor(ConsoleColor::Cyan);
				} else {
					fs::path projPath = destination / entry.path().stem();

					if (!fs::exists(projPath)) {
						fs::create_directory(projPath);
					}

					sprintf_s(command, "tar -xf \"%s\" -C \"%s\"", entry.path().string().c_str(), projPath.string().c_str());
					std::cout << "Invoking command: " << command << std::endl;
					int retCode = system(command);

					bool hasSrc = fs::exists(projPath / "src");
					bool hasRes = fs::exists(projPath / "res");
					bool hasReadme = fs::exists(projPath / "readme.txt");

					if (!hasSrc || !hasRes) {
						totalInvalid++;
						mode.SetForeColor(ConsoleColor::Red);
						std::cout << "Invalid submission: " << entry.path().stem().string() << std::endl;
						std::cout << "\tHas src?   " << (hasSrc ? "yes" : "no") << std::endl;
						std::cout << "\tHas res?   " << (hasRes ? "yes" : "no") << std::endl;
						std::cout << "\tHas readme?" << (hasReadme ? "yes" : "no") << std::endl;
						mode.SetForeColor(ConsoleColor::Cyan);
					}
				}
			}

			std::cout << "Clearing temp folder" << std::endl;
			fs::remove_all(tempPath);

			std::cout << "Invoking premake build" << std::endl;
			system("premake_build.bat");

			printf_s("\n\nRead %d submissions, %d are invalid\n", totalSubmissions, totalInvalid);
		}
	}

	std::cout << "Press any key to continue";
	_getch();
}