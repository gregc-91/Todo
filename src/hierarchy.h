#ifndef TODO_HIERARCHY_H
#define TODO_HIERARCHY_H

#include <cstddef>
#include <string>
#include <vector>

constexpr std::size_t TASK_INDENT_WIDTH = 2;

bool isTaskLine(const std::string &line);
std::size_t taskIndent(const std::string &line);
std::size_t taskDepth(const std::string &line);
std::size_t subtreeEnd(const std::vector<std::string> &lines,
					   std::size_t taskIndex);
std::vector<std::size_t> ancestorTaskIndices(
	const std::vector<std::string> &lines, std::size_t taskIndex);
void validateHierarchy(const std::vector<std::string> &lines);

#endif
