#ifndef __TODO_H__
#define __TODO_H__

#include <string>
#include <vector>

class Todo {
public:
	Todo(const std::string &filename);
	
	void addLine(unsigned index, const std::string &line);
	void removeLine(unsigned index);
	void setStatus(unsigned index, char status);
	void print();
	void commit();
	
	std::string filename;
	std::vector<std::string> lines;
};

#endif
