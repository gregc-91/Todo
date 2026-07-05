#include "todo_format.h"

#include "colour.h"
#include "todo.h"
#include "utility.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {
enum TaskType {
	Normal,
	Urgent,
	HighPriority,
	LowPriority,
	Completed,
	Suspended,
	Terminated,
	TaskTypeSize
};

const char *taskColours[TaskTypeSize] = {
	Colour::White,
	Colour::BrightRed,
	Colour::Orange,
	Colour::BrightYellow,
	Colour::BrightGreen,
	Colour::Cyan,
	Colour::BrightBlack
};
}

static std::string trimLeadingWhitespace(const std::string &value)
{
	const std::size_t start = value.find_first_not_of(" \t\f\v\n\r");
	return start == std::string::npos ? "" : value.substr(start);
}

bool isBlankLine(const std::string &line)
{
	return std::all_of(line.begin(), line.end(),
		[](unsigned char value) { return std::isspace(value); });
}

bool isProjectLine(const std::string &line)
{
	const std::string trimmed = trimLeadingWhitespace(line);
	return !trimmed.empty() && trimmed.front() == PROJECT_FILE_CHAR;
}

std::string projectNameFromLine(const std::string &line)
{
	const std::string trimmed = trimLeadingWhitespace(line);
	if (trimmed.empty() || trimmed.front() != PROJECT_FILE_CHAR) {
		return "";
	}
	return trimLeadingWhitespace(trimmed.substr(1));
}

bool projectNameMatches(const std::string &line, const std::string &project)
{
	return equalsIgnoreCase(projectNameFromLine(line), project);
}

std::string createTaskLine(char status, const std::string &text,
						   const std::string &tag)
{
	return "[" + std::string(1, status) + "] " + text +
		(tag.empty() ? "" : (" " + std::string(1, TAG_CHAR) + tag));
}

static TaskType parseTaskType(const std::string &line)
{
	if (line.size() < 4 || line[0] != '[' || line[2] != ']' ||
		line[3] != ' ') {
		throw std::runtime_error("invalid task syntax: expected '[status] task'");
	}

	switch (line[1]) {
	case '-': return Normal;
	case '!': return Urgent;
	case '^': return HighPriority;
	case 'v': return LowPriority;
	case 'x': return Completed;
	case '~': return Suspended;
	case '.': return Terminated;
	default:
		throw std::runtime_error("invalid task status");
	}
}

static bool containsTag(const std::string &line, const std::string &tag)
{
	std::stringstream stream(line);
	std::string token;
	while (std::getline(stream, token, ' ')) {
		if (token.size() > 1 && token.front() == TAG_CHAR &&
			equalsIgnoreCase(token.substr(1), tag)) {
			return true;
		}
	}
	return false;
}

bool containsAnyTag(const std::string &line,
					const std::set<std::string> &tags)
{
	for (const std::string &tag : tags) {
		if (containsTag(line, tag)) {
			return true;
		}
	}
	return false;
}

bool hasAnyStatus(const std::string &line, const std::set<char> &statuses)
{
	const std::string trimmed = trimLeadingWhitespace(line);
	if (trimmed.size() < 3 || trimmed[0] != '[' || trimmed[2] != ']') {
		return false;
	}
	return statuses.find(trimmed[1]) != statuses.end();
}

std::set<std::string> tagsFromLine(const std::string &line)
{
	std::set<std::string> tags;
	std::stringstream stream(line);
	std::string token;
	while (std::getline(stream, token, ' ')) {
		if (token.size() > 1 && token.front() == TAG_CHAR) {
			tags.insert(token.substr(1));
		}
	}
	return tags;
}

void printTodoLine(const std::string &line, uint32_t lineNumber)
{
	const std::string trimmed = trimLeadingWhitespace(line);
	const bool project = isProjectLine(line);
	const bool task = !trimmed.empty() && trimmed.front() == '[';

	std::cout << Colour::BrightBlack << std::setw(4) << lineNumber
			  << ":  " << Colour::Reset;

	if (project) {
		std::cout << Colour::BrightWhite;
	} else if (task) {
		std::cout << taskColours[parseTaskType(trimmed)];
	}

	std::cout << line << '\n' << Colour::Reset;
}
