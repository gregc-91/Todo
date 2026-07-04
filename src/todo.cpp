#include "todo.h"
#include "atomic_file.h"
#include "colour.h"
#include "command.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <filesystem>

const std::string whitespace = " \t\f\v\n\r";

std::string inline trimLeadingWhitespace(const std::string &str) {
	size_t start = str.find_first_not_of(whitespace);
	return start == std::string::npos ? "" : str.substr(start);
}

Todo::Todo(const std::string &filename) :
	filename(filename)
{
	std::ifstream file(filename);
	if (!file && std::filesystem::exists(filename)) {
		throw std::runtime_error("unable to open todo file '" + filename + "'");
	}
	for(std::string line; getline(file, line); ) {
		lines.push_back(line);
	}
}

void Todo::addLine(unsigned index, const std::string &line) {
	
	if (index > lines.size())
		throw std::runtime_error("Index out of bounds");
	
	lines.insert(lines.begin() + index, line);
}

void Todo::removeLine(unsigned index) {
	
	if (index >= lines.size())
		throw std::runtime_error("Index out of bounds");
	
	lines.erase(lines.begin() + index);
}

void Todo::setStatus(unsigned index, char status) {
	
	if (index >= lines.size())
		throw std::runtime_error("Index out of bounds");
	
	size_t pos = lines[index].find_first_of('[');
	
	if (pos == std::string::npos || pos+2 >= lines[index].length() || lines[index][pos+2] != ']')
		throw std::runtime_error("Unable to parse line as task");
	
	lines[index][pos+1] = status;
}

void Todo::print() {
	for (auto &s : lines) {
		std::cout << s << std::endl;
	}
}

void Todo::printLine(unsigned index) {
	if (index >= lines.size())
		throw std::runtime_error("Index out of bounds");

	std::string line = lines[index];

	line = trimLeadingWhitespace(line);
	bool isProject = !line.empty() && line.front() == PROJECT_FILE_CHAR;
	bool isTask = !line.empty() && line.front() == '[';

	std::cout << Colour::BrightBlack << std::setw(4) << index << ":  " << Colour::Reset;
	
	if (isProject) {
		std::cout << Colour::BrightWhite;
	} else if (isTask) {
		TaskType taskType = parseTaskType(line);
		
		std::cout << TaskColours[taskType];
	}
	
	std::cout << line << std::endl;
	
	std::cout << Colour::Reset;
}

void Todo::commit() {
	std::ostringstream contents;
	for (const std::string &line : lines) {
		contents << line << '\n';
	}
	atomicWriteFile(filename, contents.str());
}
