#include "parser.h"

#include "todo.h"
#include "utility.h"

#include <charconv>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>

namespace {
bool isValidStatus(char status)
{
	const std::string validStatuses = "-!^vx~.";
	return validStatuses.find(status) != std::string::npos;
}

char parseStatus(const char *argument)
{
	const std::string value(argument);
	char status = '\0';

	if (value.size() == 1) {
		status = value[0];
	} else if (value.size() == 3 && value.front() == '[' &&
			   value.back() == ']') {
		status = value[1];
	} else {
		throw std::invalid_argument(
			"status must be one of -, !, ^, v, x, ~, or .");
	}

	if (!isValidStatus(status)) {
		throw std::invalid_argument(
			"status must be one of -, !, ^, v, x, ~, or .");
	}
	return status;
}

uint32_t parseLineNumber(const char *argument)
{
	const std::string value(argument);
	if (value.empty()) {
		throw std::invalid_argument("line number is missing");
	}

	uint64_t parsed = 0;
	const char *begin = value.data();
	const char *end = begin + value.size();
	const auto result = std::from_chars(begin, end, parsed, 10);

	if (result.ec != std::errc() || result.ptr != end ||
		parsed > std::numeric_limits<uint32_t>::max()) {
		throw std::invalid_argument(
			"line number must be a non-negative decimal integer");
	}
	return static_cast<uint32_t>(parsed);
}

void setListMode(Command &command, ListMode mode)
{
	if (command.list.mode != ListMode::Tasks && command.list.mode != mode) {
		throw std::invalid_argument(
			"list can show projects or tags, but not both");
	}
	command.list.mode = mode;
}

Command parseCommandType(const char *argument)
{
	struct Candidate {
		const char *name;
		CommandType type;
	};
	const Candidate candidates[] = {
		{"list", CommandType::List},
		{"add", CommandType::Add},
		{"remove", CommandType::Remove},
		{"do", CommandType::Doo},
		{"set", CommandType::Set},
		{"undo", CommandType::Undo},
		{"tidy", CommandType::Tidy}
	};

	const std::string input(argument);
	const Candidate *closest = &candidates[0];
	std::size_t closestDistance = editDistance(input, closest->name);

	for (const Candidate &candidate : candidates) {
		if (equalsIgnoreCase(input, candidate.name)) {
			return Command(candidate.type);
		}

		const std::size_t distance = editDistance(input, candidate.name);
		if (distance < closestDistance) {
			closest = &candidate;
			closestDistance = distance;
		}
	}

	throw std::invalid_argument(
		"unknown command '" + input + "'; did you mean '" +
		closest->name + "'?");
}

void parseListCommand(int argc, char **argv, Command &command)
{
	for (int i = 0; i < argc; ++i) {
		const std::string argument(argv[i]);
		if (argument.empty()) {
			throw std::invalid_argument("empty list argument");
		}

		switch (argument.front()) {
		case PROJECT_ARGUMENT_CHAR:
			if (argument.size() == 1) {
				setListMode(command, ListMode::Projects);
			} else {
				if (!command.list.project.empty()) {
					throw std::invalid_argument("duplicate project filter");
				}
				command.list.project = argument.substr(1);
			}
			break;
		case TAG_CHAR:
			if (argument.size() == 1) {
				setListMode(command, ListMode::Tags);
			} else {
				command.list.tags.insert(argument.substr(1));
			}
			break;
		case '[':
			if (argument.size() < 3 || argument.back() != ']') {
				throw std::invalid_argument(
					"status filter must look like '[x]' or '[-!^v]'");
			}
			for (std::size_t j = 1; j + 1 < argument.size(); ++j) {
				if (!isValidStatus(argument[j])) {
					throw std::invalid_argument(
						"status filter contains an invalid status");
				}
				command.list.statuses.insert(argument[j]);
			}
			break;
		default:
			throw std::invalid_argument(
				"unknown list argument '" + argument + "'");
		}
	}
}

void parseAddCommand(int argc, char **argv, Command &command)
{
	for (int i = 0; i < argc; ++i) {
		const std::string argument(argv[i]);
		if (argument.empty()) {
			throw std::invalid_argument("empty add argument");
		}

		if (argument.front() == PROJECT_ARGUMENT_CHAR) {
			if (argument.size() == 1) {
				throw std::invalid_argument("project name is missing");
			}
			if (!command.add.project.empty()) {
				throw std::invalid_argument("duplicate project");
			}
			command.add.project = argument.substr(1);
		} else if (argument.front() == TAG_CHAR) {
			if (argument.size() == 1) {
				throw std::invalid_argument("tag name is missing");
			}
			if (!command.add.tag.empty()) {
				throw std::invalid_argument("duplicate tag");
			}
			command.add.tag = argument.substr(1);
		} else {
			if (!command.add.task.empty()) {
				throw std::invalid_argument(
					"task text must be passed as one quoted argument");
			}
			command.add.task = argument;
		}
	}

	if (command.add.task.empty()) {
		throw std::invalid_argument("task text is missing");
	}
}

void requireArgumentCount(const char *command, int actual, int expected)
{
	if (actual != expected) {
		throw std::invalid_argument(
			std::string(command) + " expects " +
			std::to_string(expected) + " argument" +
			(expected == 1 ? "" : "s"));
	}
}
}

Command parseCommand(int argc, char **argv)
{
	if (argc < 2) {
		throw std::invalid_argument("no command supplied; use --help");
	}

	Command command = parseCommandType(argv[1]);
	const int argumentCount = argc - 2;
	char **arguments = argv + 2;

	switch (command.type()) {
	case CommandType::List:
		parseListCommand(argumentCount, arguments, command);
		break;
	case CommandType::Add:
		parseAddCommand(argumentCount, arguments, command);
		break;
	case CommandType::Remove:
		requireArgumentCount("remove", argumentCount, 1);
		command.remove.index = parseLineNumber(arguments[0]);
		break;
	case CommandType::Doo:
		requireArgumentCount("do", argumentCount, 1);
		command.doo.index = parseLineNumber(arguments[0]);
		break;
	case CommandType::Set:
		requireArgumentCount("set", argumentCount, 2);
		command.set.status = parseStatus(arguments[0]);
		command.set.index = parseLineNumber(arguments[1]);
		break;
	case CommandType::Undo:
		requireArgumentCount("undo", argumentCount, 0);
		break;
	case CommandType::Tidy:
		requireArgumentCount("tidy", argumentCount, 0);
		break;
	default:
		throw std::invalid_argument("unsupported command");
	}

	return command;
}
