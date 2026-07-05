#ifndef TODO_FORMAT_H
#define TODO_FORMAT_H

#include <cstdint>
#include <set>
#include <string>

bool isBlankLine(const std::string &line);
bool isProjectLine(const std::string &line);
std::string projectNameFromLine(const std::string &line);
bool projectNameMatches(const std::string &line, const std::string &project);

std::string createTaskLine(char status, const std::string &text,
						   const std::string &tag);
bool containsAnyTag(const std::string &line,
					const std::set<std::string> &tags);
bool hasAnyStatus(const std::string &line, const std::set<char> &statuses);
std::set<std::string> tagsFromLine(const std::string &line);

void printTodoLine(const std::string &line, uint32_t lineNumber);

#endif
