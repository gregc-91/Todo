#ifndef __TODO_H__
#define __TODO_H__

#include <string>
#include <vector>

#define PROJECT_CHAR '+'
#define TAG_CHAR '@'

class Todo {
public:
	Todo(const std::string &filename);
	
	void addLine(unsigned index, const std::string &line);
	void removeLine(unsigned index);
	void setStatus(unsigned index, char status);
	void print();
	void printLine(unsigned index);
	void commit();
	
	std::string filename;
	std::vector<std::string> lines;
};

#endif
