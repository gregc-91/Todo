#include "todo.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>

Todo::Todo(const std::string &filename) :
	filename(filename) 
{
	std::ifstream file(filename);
	for(std::string line; getline(file, line); ) {
		lines.push_back(line);
	}
	file.close();
}

void Todo::addLine(unsigned index, const std::string &line) {
	
	if (index > lines.size())
		throw std::runtime_error("Index out of bounds");
	
	lines.insert(lines.begin() + index, line);
}

void Todo::removeLine(unsigned index) {
	
	if (index > lines.size())
		throw std::runtime_error("Index out of bounds");
	
	lines.erase(lines.begin() + index);
}

void Todo::setStatus(unsigned index, char status) {
	
	if (index > lines.size())
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

void Todo::commit() {
	std::ofstream file(filename);
	for (auto &s : lines) {
		file << s << std::endl;
	}
	file.close();
}