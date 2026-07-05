#include "hierarchy.h"

#include "todo_format.h"

#include <stdexcept>

namespace {
std::size_t leadingSpaces(const std::string &line)
{
	std::size_t spaces = 0;
	for (char value : line) {
		if (value == ' ') {
			++spaces;
		} else if (value == '\t') {
			throw std::runtime_error(
				"tabs are not allowed for hierarchy indentation");
		} else {
			break;
		}
	}
	return spaces;
}

bool isValidStatus(char status)
{
	return std::string("-!^vx~.").find(status) != std::string::npos;
}
}

bool isTaskLine(const std::string &line)
{
	const std::size_t first = line.find_first_not_of(" \t");
	return first != std::string::npos && first + 3 < line.size() &&
		line[first] == '[' && line[first + 2] == ']' &&
		line[first + 3] == ' ';
}

std::size_t taskIndent(const std::string &line)
{
	if (!isTaskLine(line)) {
		throw std::runtime_error("line is not a task");
	}

	const std::size_t indentation = leadingSpaces(line);
	if (indentation % TASK_INDENT_WIDTH != 0) {
		throw std::runtime_error(
			"task indentation must use multiples of two spaces");
	}
	return indentation;
}

std::size_t taskDepth(const std::string &line)
{
	return taskIndent(line) / TASK_INDENT_WIDTH;
}

std::size_t subtreeEnd(const std::vector<std::string> &lines,
					   std::size_t taskIndex)
{
	if (taskIndex >= lines.size()) {
		throw std::runtime_error("Index out of bounds");
	}

	const std::size_t parentIndent = taskIndent(lines[taskIndex]);
	std::size_t firstPendingBlank = lines.size();
	for (std::size_t i = taskIndex + 1; i < lines.size(); ++i) {
		if (isBlankLine(lines[i])) {
			if (firstPendingBlank == lines.size()) {
				firstPendingBlank = i;
			}
			continue;
		}
		if (isProjectLine(lines[i])) {
			return firstPendingBlank == lines.size() ? i : firstPendingBlank;
		}
		if (isTaskLine(lines[i]) && taskIndent(lines[i]) <= parentIndent) {
			return firstPendingBlank == lines.size() ? i : firstPendingBlank;
		}

		const std::size_t indentation = leadingSpaces(lines[i]);
		if (indentation <= parentIndent) {
			return firstPendingBlank == lines.size() ? i : firstPendingBlank;
		}
		firstPendingBlank = lines.size();
	}
	return firstPendingBlank;
}

std::vector<std::size_t> ancestorTaskIndices(
	const std::vector<std::string> &lines, std::size_t taskIndex)
{
	if (taskIndex >= lines.size()) {
		throw std::runtime_error("Index out of bounds");
	}

	std::size_t depth = taskDepth(lines[taskIndex]);
	std::vector<std::size_t> ancestors;
	if (depth == 0) {
		return ancestors;
	}

	for (std::size_t i = taskIndex; i-- > 0 && depth > 0;) {
		if (isProjectLine(lines[i])) {
			break;
		}
		if (isTaskLine(lines[i]) && taskDepth(lines[i]) + 1 == depth) {
			ancestors.push_back(i);
			depth -= 1;
		}
	}
	return ancestors;
}

void validateHierarchy(const std::vector<std::string> &lines)
{
	std::vector<bool> activeDepths;

	for (std::size_t i = 0; i < lines.size(); ++i) {
		if (isProjectLine(lines[i])) {
			try {
				if (leadingSpaces(lines[i]) != 0) {
					throw std::runtime_error(
						"project headings must not be indented");
				}
			} catch (const std::runtime_error &error) {
				throw std::runtime_error(
					"line " + std::to_string(i) + ": " + error.what());
			}
			activeDepths.clear();
			continue;
		}

		const std::size_t first = lines[i].find_first_not_of(" \t");
		if (first != std::string::npos && lines[i][first] == '[' &&
			!isTaskLine(lines[i])) {
			throw std::runtime_error(
				"line " + std::to_string(i) +
				": invalid task syntax; expected '[status] task'");
		}

		if (!isTaskLine(lines[i])) {
			if (!isBlankLine(lines[i])) {
				try {
					const std::size_t indentation = leadingSpaces(lines[i]);
					if (indentation % TASK_INDENT_WIDTH != 0) {
						throw std::runtime_error(
							"indentation must use multiples of two spaces");
					}
					const std::size_t retainedDepths =
						indentation / TASK_INDENT_WIDTH;
					if (retainedDepths < activeDepths.size()) {
						activeDepths.resize(retainedDepths);
					}
				} catch (const std::runtime_error &error) {
					throw std::runtime_error(
						"line " + std::to_string(i) + ": " + error.what());
				}
			}
			continue;
		}

		try {
			if (!isValidStatus(lines[i][first + 1])) {
				throw std::runtime_error("invalid task status");
			}

			const std::size_t depth = taskDepth(lines[i]);
			if (depth > 0 &&
				(activeDepths.size() < depth ||
				 !activeDepths[depth - 1])) {
				throw std::runtime_error(
					"subtask has no parent at the previous indentation level");
			}

			activeDepths.resize(depth + 1);
			activeDepths[depth] = true;
		} catch (const std::runtime_error &error) {
			throw std::runtime_error(
				"line " + std::to_string(i) + ": " + error.what());
		}
	}
}
