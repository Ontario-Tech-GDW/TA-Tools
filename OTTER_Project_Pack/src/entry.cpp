// OTTER_Project_Pack.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <iosfwd>
#include <fstream>

#include <Windows.h>
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

int GetSelectionIndex(const std::string& prompt, int size, int min = 0) {
	int selection = 0;
	while (true) {
		std::cout << prompt;
		if (!(std::cin >> selection)) {
			std::cout << "Invalid value, must be a number between " << min + 1 << " and " << size << " (inclusive)" << std::endl;
			std::cin.clear();
			std::cin.ignore(256, '\n');
		} else if (selection > min && selection <= size) {
			std::cin.ignore(256, '\n');
			return selection - 1;
		} else {
			std::cout << "Invalid value, must be between " << min + 1 << " and " << size << " (inclusive)" << std::endl;
			std::cin.clear();
			std::cin.ignore(256, '\n');
		}
	}
}

uint64_t GetStudentId(const std::string& prompt) {
	uint64_t selection = 0;
	while (true) {
		std::cout << prompt;
		if (!(std::cin >> selection)) {
			std::cout << "Invalid value, must be a number with 9 digits" << std::endl;
			std::cin.clear();
			std::cin.ignore(256, '\n');
		} else if (((uint64_t)log10((double)selection) + 1) == 9) {
			std::cin.ignore(256, '\n');
			return selection;
		} else {
			std::cout << "Invalid value, must have 9 digits" << std::endl;
			std::cin.clear();
			std::cin.ignore(256, '\n');
		}
	}
}

std::string ReadString(const std::string& prompt, bool required = true) {
	std::string result;
	do {
		std::cout << prompt;
		getline(std::cin, result);
		if (result.empty() & required) {
			std::cout << "This is a required field, please enter a value" << std::endl;
		}
	} while (result.empty() & required);
	return result;
}

std::string Capitalize(const std::string& value) {
	std::string result = value;
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
		return std::tolower(c);
	});
	if (result.length() > 0) {
		result[0] = std::toupper(result[0]);
	}
	return result;
}

struct StudentInfo {
	std::string FirstName;
	std::string LastName;
	uint64_t    StudentId;
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

	std::cout << "=============================================================\n";
	std::cout << "===            Welcome to OTTER Project Pack              ===\n";
	std::cout << "=============================================================\n";
	std::cout << "===   This tool is used to zip projects created in OTTER  ===\n";
	std::cout << "===               for assignment submission               ===\n";
	std::cout << "=============================================================\n";

	if (!std::filesystem::exists("projects")) {
		std::filesystem::create_directory("projects");
		mode.SetForeColor(ConsoleColor::Yellow);
		std::cout << "[WARN] Projects directory not found, creating..." << std::endl;
		mode.SetForeColor(ConsoleColor::Cyan);
	}

	std::cout << " Available projects:" << std::endl;

	std::vector<std::filesystem::path> projects;
	int ix = 1;
	for (const auto& entry : std::filesystem::directory_iterator("projects")) {
		if (entry.is_directory()) {
			projects.push_back(entry.path());
			std::cout << ix++ << ": " << entry.path().filename().string() << std::endl;
		}
	}

	int projectId = GetSelectionIndex("Select project ID: ", projects.size());
	std::filesystem::path projPath = projects[projectId];
	std::cout << "Using project \"" << projPath.string() << "\"" << std::endl;

	std::cout << "=============================================================\n";

	std::string assignmentName = ReadString("Enter assignment name (leave blank to use project name): ", false);
	if (assignmentName.empty()) {
		assignmentName = projPath.filename().string();
	}

	std::cout << "=============================================================\n";

	std::vector<StudentInfo> students;
	int numStudents = GetSelectionIndex("Enter # of students that worked on this assignment: ", 8) + 1;
	std::cout << "=============================================================\n";

	students.resize(numStudents);
	for (int ix = 0; ix < numStudents; ix++) {
		std::cout << "Entering info for student " << (ix + 1) << std::endl;
		students[ix].FirstName = Capitalize(ReadString("First Name: "));
		students[ix].LastName  = Capitalize(ReadString("Last Name:  ", false));
		students[ix].StudentId = GetStudentId("Student ID: ");
		std::cout << "=============================================================\n";
	}

	std::stringstream outFilename;
	outFilename << assignmentName;
	for (int ix = 0; ix < students.size(); ix++) {
		outFilename << "_";
		outFilename << students[ix].LastName << students[ix].FirstName;
	}
	std::string outFilenameStr = outFilename.str();
	std::filesystem::path outFilenamePath = outFilename.str();

	std::cout << "Generating readme..." << std::endl;
	std::ofstream readme((projPath / "readme.txt").c_str(), std::ios::out | std::ios::app);
	readme << "=============================================================\n";
	readme << assignmentName << std::endl;
	readme << "=============================================================\n";
	for (int ix = 0; ix < students.size(); ix++) {
		readme << students[ix].FirstName << " " << students[ix].LastName << " (" << students[ix].StudentId << ")" << std::endl;
	}
	readme.close();


	static char command[4096];
	sprintf_s(command, "tar -acf \"%s.zip\" -C \"%s\" --exclude='*.vcxproj' --exclude='*.filters' --exclude='*.user' src res readme.txt", outFilenameStr.c_str(), projPath.string().c_str());
	std::cout << "Invoking command: " << command << std::endl;
	int retCode = system(command);

	if (retCode != 0) {
		mode.SetForeColor(Red);
		std::cout << "Failed to generate zip file!" << std::endl;
		mode.Reset();
	}

	std::cout << "Cleaning temp files" << std::endl;
	std::filesystem::remove(projPath / "readme.txt");

	mode.Reset();

	std::cout << "Press any key to continue";
	_getch();

	return retCode;
}
