#include "todo.h"
#include "atomic_file.h"
#include "todo_format.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

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

void Todo::removeLines(unsigned first, unsigned last) {
	if (first > last || last > lines.size()) {
		throw std::runtime_error("Index out of bounds");
	}
	lines.erase(lines.begin() + first, lines.begin() + last);
}

void Todo::setStatus(unsigned index, char status) {
	
	if (index >= lines.size())
		throw std::runtime_error("Index out of bounds");
	
	size_t pos = lines[index].find_first_of('[');
	
	if (pos == std::string::npos || pos+2 >= lines[index].length() || lines[index][pos+2] != ']')
		throw std::runtime_error("Unable to parse line as task");
	
	lines[index][pos+1] = status;
}

void Todo::printLine(unsigned index) {
	if (index >= lines.size())
		throw std::runtime_error("Index out of bounds");
	printTodoLine(lines[index], index);
}

void Todo::commit() {
	std::ostringstream contents;
	for (const std::string &line : lines) {
		contents << line << '\n';
	}
	atomicWriteFile(filename, contents.str());
}
