#ifndef TODO_TODO_H
#define TODO_TODO_H

#include <string>
#include <vector>

constexpr char PROJECT_ARGUMENT_CHAR = '+';
constexpr char PROJECT_FILE_CHAR = '#';
constexpr char TAG_CHAR = '@';

class Todo {
public:
	Todo(const std::string &filename);
	
	void addLine(unsigned index, const std::string &line);
	void removeLine(unsigned index);
	void setStatus(unsigned index, char status);
	void print();
	void printLine(unsigned index);
	void commit();
	void backup();
	void restore();
	
	std::string filename;
	std::vector<std::string> lines;
};

#endif
