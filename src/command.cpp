
#include "command.h"

#include "colour.h"
#include "history.h"
#include "todo.h"
#include "utility.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <set>

const char* CommandStrings[CommandType::CommandTypeSize] = {
	"List",
	"Add",
	"Remove",
	"Do",
	"Set",
	"Undo",
	"Tidy",
	"Restore",
	"None"
};

const char* TaskColours[TaskType::TaskTypeSize] = {
	Colour::White,
	Colour::BrightRed,
	Colour::Orange,
	Colour::BrightYellow,
	Colour::BrightGreen,
	Colour::Cyan,
	Colour::BrightBlack,
};

const std::string whitespace = " \t\f\v\n\r";

std::string inline trimLeadingWhitespace(const std::string &str) {
	size_t start = str.find_first_not_of(whitespace);
	return start == std::string::npos ? "" : str.substr(start);
}

bool isBlank(const std::string &line) {
	return std::all_of(line.begin(), line.end(),
		[](unsigned char value) { return std::isspace(value); });
}

std::string createTaskString(const char status, std::string line, std::string tag) {
	return "[" + std::string(1, status) + "] " + line +
		(tag.empty() ? "" : (" " + std::string(1, TAG_CHAR) + tag));
}

bool isAnyProject(const std::string &str) {
	std::string trimmedLine = trimLeadingWhitespace(str);
	return !trimmedLine.empty() && trimmedLine.front() == PROJECT_FILE_CHAR;
}

std::string projectName(const std::string &str) {
	std::string line = trimLeadingWhitespace(str);
	if (line.empty() || line.front() != PROJECT_FILE_CHAR) {
		return "";
	}
	return trimLeadingWhitespace(line.substr(1));
}

bool isProject(const std::string &str, const std::string &project) {
	return equalsIgnoreCase(projectName(str), project);
}

bool containsTag(const std::string &str, const std::string &tag) {
	std::stringstream stream(str);
	std::string intermediate;
	while(getline(stream, intermediate, ' ')) {
		if (intermediate.size() > 1 && intermediate.front() == TAG_CHAR &&
			equalsIgnoreCase(intermediate.substr(1), tag)) {
			return true;
		}
	}
	return false;
}

bool containsTagFromSet(const std::string &str, const std::set<std::string> &tags) {
	for (auto tag: tags) {
		if (containsTag(str, tag)) return true;
	}
	return false;
}

bool hasStatus(const std::string &str, const char status) {
	if (str.size() >= 3 && str[0] == '[' && str[1] == status && str[2] == ']') {
		return true;
	}
	return false;
}

bool hasStatusFromSet(const std::string &str, const std::set<char> &statuses) {
	for (auto status : statuses) {
		if (hasStatus(str, status)) return true;
	}
	return false;
}

int findLastOfProject(const Todo &todo, const std::string &project) {
	
	if (project.empty())
		return todo.lines.size();
	
	bool matchProject = false;
	unsigned lastNonEmpty = 0;
	for (unsigned i = 0; i < todo.lines.size(); i++) {
		std::string line = trimLeadingWhitespace(todo.lines[i]);
		if (isAnyProject(line)) {
			if (isProject(todo.lines[i], project)) matchProject = true;
			else if (matchProject) return lastNonEmpty+1;
		}
		if (!line.empty()) lastNonEmpty = i;
	}

	if (matchProject) {
		return todo.lines.size();
	}
	else {
		return -1;
	}
}

TaskType parseTaskType(const std::string &line) {
	if (line.size() < 4 || line[0] != '[' || line[2] != ']' ||
		line[3] != ' ') {
		throw std::runtime_error("invalid task syntax: expected '[status] task'");
	}
	
	switch (line[1]) {
	case '-': return TaskType::Normal;
	case '!': return TaskType::Urgent;
	case '^': return TaskType::HighPriority;
	case 'v': return TaskType::LowPriority;
	case 'x': return TaskType::Completed;
	case '~': return TaskType::Suspended;
	case '.': return TaskType::Terminated;
	default:
		throw std::runtime_error("Found invalid task status");
	}
}

std::set<std::string> getTags(std::string line) {
	std::set<std::string> tags;
	std::stringstream stream(line);
	std::string intermediate;
	while(getline(stream, intermediate, ' ')) {
		if (intermediate.length() && intermediate[0] == TAG_CHAR) {
			if (intermediate.length() > 1) tags.insert(intermediate.substr(1));
		}
	}
	return tags;
}

struct Line {
	std::string line;
	uint32_t no;
};

struct Project {
	std::string name;
	uint32_t no;
	std::string heading;
	std::vector<Line> lines;
};

void printLine(std::string line, uint32_t lineNo) {

	std::string trimmedLine = trimLeadingWhitespace(line);
	bool isProject = !trimmedLine.empty() &&
		trimmedLine.front() == PROJECT_FILE_CHAR;
	bool isTask = !trimmedLine.empty() && trimmedLine.front() == '[';

	std::cout << Colour::BrightBlack << std::setw(4) << lineNo << ":  " << Colour::Reset;
		
	if (isProject) {
		std::cout << Colour::BrightWhite;
	} else if (isTask) {
		TaskType taskType = parseTaskType(trimmedLine);
		
		std::cout << TaskColours[taskType];
		
		//line = line.subtr(3);
	}
	
	std::cout << line << std::endl;
	
	std::cout << Colour::Reset;
};

bool projectMatches(const Project &project, const Command &command)
{
	return command.list.project.empty() ||
		equalsIgnoreCase(project.name, command.list.project);
}

bool lineMatches(const std::string &line, const Command &command)
{
	if (!command.list.tags.empty() &&
		!containsTagFromSet(line, command.list.tags)) {
		return false;
	}
	if (!command.list.statuses.empty() &&
		!hasStatusFromSet(trimLeadingWhitespace(line),
			command.list.statuses)) {
		return false;
	}
	return true;
}

static void executeListCommand(const Todo &todo, const Command &command) {
	if (DEBUG_PRINT) printf("Executing list command.\n");

	bool filterTag     = !command.list.tags.empty();
	bool filterStatus  = !command.list.statuses.empty();
	std::set<std::string> tagsInFile;
	std::vector<Project> projects;

	unsigned lineNo = 0;
	for (const std::string &line : todo.lines) {
		std::string trimmedLine = trimLeadingWhitespace(line);
		if (isAnyProject(trimmedLine)) {
			projects.push_back({
				projectName(trimmedLine), lineNo, line, std::vector<Line>()
			});
		} else {
			if (projects.empty()) {
				projects.push_back({
					std::string(), 0, std::string(), std::vector<Line>()
				});
			}
			projects.back().lines.push_back({line, lineNo});
		}
		++lineNo;
	}

	if (command.list.mode == ListMode::Projects) {
		for (const Project &project : projects) {
			if (project.heading.empty() || !projectMatches(project, command)) {
				continue;
			}
			if ((filterTag || filterStatus) &&
				std::none_of(project.lines.begin(), project.lines.end(),
					[&command](const Line &line) {
						return lineMatches(line.line, command);
					})) {
				continue;
			}
			printLine(project.heading, project.no);
		}
		return;
	}

	if (command.list.mode == ListMode::Tags) {
		for (const Project &project : projects) {
			if (!projectMatches(project, command)) {
				continue;
			}
			for (const Line &line : project.lines) {
				if (filterStatus &&
					!hasStatusFromSet(trimLeadingWhitespace(line.line),
						command.list.statuses)) {
					continue;
				}
				for (const std::string &tag : getTags(line.line)) {
					if (!filterTag ||
						containsTagFromSet(
							std::string(1, TAG_CHAR) + tag,
							command.list.tags)) {
						tagsInFile.insert(tag);
					}
				}
			}
		}
		for (const std::string &tag : tagsInFile) {
			std::cout << "  " << TAG_CHAR << tag << std::endl;
		}
		return;
	}

	for (const Project &project : projects) {
		if (!projectMatches(project, command)) {
			continue;
		}

		std::vector<Line> matchingLines;
		for (const Line &line : project.lines) {
			if ((!filterTag && !filterStatus) ||
				lineMatches(line.line, command)) {
				matchingLines.push_back(line);
			}
		}

		if (matchingLines.empty() && (filterTag || filterStatus)) {
			continue;
		}
		if (!project.heading.empty()) {
			printLine(project.heading, project.no);
		}
		for (const Line &line : matchingLines) {
			printLine(line.line, line.no);
		}
	}
}

static void executeAddCommand(Todo& todo, Command &command) {
	if (DEBUG_PRINT) printf("Executing add command.\n");
	
	std::string taskString = createTaskString('-', command.add.task, command.add.tag);
	
	int index = findLastOfProject(todo, command.add.project);

	if (index == -1) { // Project doesn't exist, add it first
		if (!todo.lines.empty() && !isBlank(todo.lines.back())) {
			todo.addLine(todo.lines.size(), std::string(""));
		}
		todo.addLine(todo.lines.size(),
			std::string(1, PROJECT_FILE_CHAR) + " " + command.add.project);
		index = todo.lines.size();
	}

	command.add.index = index;
	todo.addLine(command.add.index, taskString);
	todo.commit();
	todo.printLine(command.add.index);
}

static void executeRemoveCommand(Todo& todo, const Command &command) {
	if (DEBUG_PRINT) printf("Executing remove command.\n");
	
	todo.printLine(command.remove.index);
	todo.removeLine(command.remove.index);
	todo.commit();
}

static void executeDooCommand(Todo& todo, const Command &command) {
	if (DEBUG_PRINT) printf("Executing do command.\n");
	
	todo.setStatus(command.doo.index, command.doo.status);
	todo.commit();
	todo.printLine(command.doo.index);
}

static void executeSetCommand(Todo& todo, const Command &command) {
	if (DEBUG_PRINT) printf("Executing set command.\n");
	
	todo.setStatus(command.set.index, command.set.status);
	todo.commit();
	todo.printLine(command.set.index);
}

static void executeUndoCommand(Todo& todo, const Command &) {
	if (DEBUG_PRINT) printf("Executing undo command.\n");

	History history("history.txt");
	HistoryEntry entry = history.last();
	const std::vector<std::string> linesBeforeUndo = todo.lines;

	entry.command.print();
	entry.inverse.print();
	executeCommand(todo, entry.inverse);

	history.pop();
	try {
		history.commit();
	} catch (...) {
		todo.lines = linesBeforeUndo;
		todo.commit();
		throw;
	}
}

static void executeTidyCommand(Todo &todo, const Command &) {
	if (DEBUG_PRINT) printf("Executing tidy command.\n");

	for(size_t i = 1; i < todo.lines.size(); i++) {
		// Remove duplicate blank lines
		if (isBlank(todo.lines[i-1]) && isBlank(todo.lines[i])) {
			todo.removeLine(i--);
			continue;
		}

		// Remove blank lines which aren't immediately before a project
		if (isBlank(todo.lines[i-1]) &&
			!isAnyProject(todo.lines[i])) {
			todo.removeLine(--i);
			continue;
		}
	}

	// Add a blank line before a project
	for(size_t i = 1; i < todo.lines.size(); i++) {
		if (!isBlank(todo.lines[i-1]) &&
			isAnyProject(todo.lines[i])) {
			todo.addLine(i, "");
		}
	}

	todo.commit();
}
static void executeRestoreCommand(Todo &todo, const Command &command) {
	todo.lines = command.restore.lines;
	todo.commit();
}

void executeCommand(Todo &todo, Command &command) {
	
	switch (command.type()) {
	case CommandType::List:
		{
			executeListCommand(todo, command);
		} break;
	case CommandType::Add:
		{
			executeAddCommand(todo, command);
		} break;
	case CommandType::Remove:
		{
			executeRemoveCommand(todo, command);
		} break;
	case CommandType::Doo:
		{
			executeDooCommand(todo, command);
		} break;
	case CommandType::Set:
		{
			executeSetCommand(todo, command);
		} break;
	case CommandType::Undo:
		{
			executeUndoCommand(todo, command);
		} break;
	case CommandType::Tidy:
		{
			executeTidyCommand(todo, command);
		} break;
	case CommandType::Restore:
		{
			executeRestoreCommand(todo, command);
		} break;
	default: break;
	}
}

Command inverseCommand(const Todo &todo, const Command &command)
{
	switch (command.type()) {
	case CommandType::Add:
	case CommandType::Remove:
	case CommandType::Doo:
	case CommandType::Set:
	case CommandType::Tidy:
		{
			Command inverse(CommandType::Restore);
			inverse.restore.lines = todo.lines;
			return inverse;
		}
	default: break;
	}
	
	return Command();
}

void serialise_string(std::ostream &os, const std::string &str)
{
	os << str.length() << ',';
	if (str.length()) {
		os << str;
	}
	os << ',';
}

void serialise_set(std::ostream &os, const std::set<std::string> &set)
{
	os << set.size() << ',';
	for (auto &str : set) {
		serialise_string(os, str);
	}
}

void serialise_set(std::ostream &os, const std::set<char> &set)
{
	os << set.size() << ',';
	for (auto &chr : set) {
		os << chr;
	}
	os << ',';
}

void deserialise_string(std::istream &is, std::string &str)
{
	size_t length = 0;
	char delimiter = '\0';
	if (!(is >> length >> delimiter) || delimiter != ',') {
		throw std::runtime_error("malformed string in command history");
	}

	str.resize(length);
	if (length > 0 &&
		!is.read(&str[0], static_cast<std::streamsize>(length))) {
		throw std::runtime_error("truncated string in command history");
	}
	if (!is.get(delimiter) || delimiter != ',') {
		throw std::runtime_error("malformed string in command history");
	}
}

void deserialise_set(std::istream &is, std::set<std::string> &set)
{
	size_t length = 0;
	char delimiter = '\0';
	if (!(is >> length >> delimiter) || delimiter != ',') {
		throw std::runtime_error("malformed set in command history");
	}
	set.clear();
	for (unsigned i = 0; i < length; i++) {
		std::string str;
		deserialise_string(is, str);
		set.insert(str);
	}
}

void deserialise_set(std::istream &is, std::set<char> &set)
{
	size_t length = 0;
	char delimiter = '\0';
	if (!(is >> length >> delimiter) || delimiter != ',') {
		throw std::runtime_error("malformed set in command history");
	}
	set.clear();
	for (unsigned i = 0; i < length; i++) {
		char value = '\0';
		if (!is.get(value)) {
			throw std::runtime_error("truncated set in command history");
		}
		set.insert(value);
	}
	if (!is.get(delimiter) || delimiter != ',') {
		throw std::runtime_error("malformed set in command history");
	}
}

template <typename T>
void deserialise_value(std::istream &is, T &value)
{
	char delimiter = '\0';
	if (!(is >> value >> delimiter) || delimiter != ',') {
		throw std::runtime_error("malformed value in command history");
	}
}

void Command::serialise(std::ostream &os) const
{
	os << ct << ',';
	
	switch (ct) {
	case CommandType::List:
	{
		os << list.mode << ',';
		serialise_string(os, list.project);
		serialise_set(os, list.tags);
		serialise_set(os, list.statuses);
		break;
	}
	case CommandType::Add:
	{
		serialise_string(os, add.project);
		serialise_string(os, add.tag);
		serialise_string(os, add.task);
		os << add.index << ',';
		break;
	}
	case CommandType::Remove:
	{
		serialise_string(os, remove.project);
		serialise_string(os, remove.tag);
		os << remove.index << ',';
		break;
	}
	case CommandType::Doo:
	{
		serialise_string(os, doo.project);
		serialise_string(os, doo.tag);
		os << doo.index << ',';
		os << doo.status << ',';
		break;
	}
	case CommandType::Set:
	{
		os << set.index << ',';
		os << set.status << ',';
		break;
	}
	case CommandType::Tidy:
	{
		break;
	}
	case CommandType::Restore:
	{
		os << restore.lines.size() << ',';
		for (const std::string &line : restore.lines) {
			serialise_string(os, line);
		}
		break;
	}
	default: break;
	}
}

void Command::deserialise(std::istream &is)
{
	char tmp;
	int type;
	if (!(is >> type >> tmp) || tmp != ',' ||
		type < CommandType::List || type > CommandType::None) {
		throw std::runtime_error("invalid command type in history");
	}
	ct = static_cast<CommandType>(type);

	switch (ct) {
	case CommandType::List:
	{
		list = ListCommand();
		deserialise_value(is, list.mode);
		deserialise_string(is, list.project);
		deserialise_set(is, list.tags);
		deserialise_set(is, list.statuses);
		break;
	}
	case CommandType::Add:
	{
		add = AddCommand();
		deserialise_string(is, add.project);
		deserialise_string(is, add.tag);
		deserialise_string(is, add.task);
		deserialise_value(is, add.index);
		break;
	}
	case CommandType::Remove:
	{
		remove = RemoveCommand();
		deserialise_string(is, remove.project);
		deserialise_string(is, remove.tag);
		deserialise_value(is, remove.index);
		break;
	}
	case CommandType::Doo:
	{
		doo = DoCommand();
		deserialise_string(is, doo.project);
		deserialise_string(is, doo.tag);
		deserialise_value(is, doo.index);
		deserialise_value(is, doo.status);
		break;
	}
	case CommandType::Set:
	{
		set = SetCommand();
		deserialise_value(is, set.index);
		deserialise_value(is, set.status);
		break;
	}
	case CommandType::Tidy:
	{
		break;
	}
	case CommandType::Restore:
	{
		restore = RestoreCommand();
		std::size_t lineCount = 0;
		deserialise_value(is, lineCount);
		restore.lines.reserve(lineCount);
		for (std::size_t i = 0; i < lineCount; ++i) {
			std::string line;
			deserialise_string(is, line);
			restore.lines.push_back(line);
		}
		break;
	}
	default:	
		throw std::runtime_error("Unexpected command type in command history deserialisation");
	}
}

bool Command::shouldUpdateHistory() const
{
	switch (ct) {
	case CommandType::Add:
	case CommandType::Remove:
	case CommandType::Doo:
	case CommandType::Set:
	case CommandType::Tidy:
		return true;
	default:
		return false;
	}
}

void Command::print() const
{
	if (!DEBUG_PRINT) return;

	printf("{Type: %s, ", CommandStrings[ct]);
	
	switch (ct) {
	case CommandType::List:
	{
		printf("Mode: %d, ", list.mode);
		printf("Project: %s, ", list.project.c_str());
		printf("Tags: {");
		for (auto tag : list.tags) printf("%s, ", tag.c_str());
		printf("}, ");
		printf("Statuses: [");
		for (auto status : list.statuses) printf("%c", status);
		printf("]\n");
		break;
	}
	case CommandType::Add:
	{
		printf("Project: %s, ", add.project.c_str());
		printf("Tag: %s, ", add.tag.c_str());
		printf("Task: %s, ", add.task.c_str());
		printf("Index: %u}\n", add.index);
		break;
	}
	case CommandType::Remove:
	{
		printf("Project: %s, ", remove.project.c_str());
		printf("Tag: %s, ", remove.tag.c_str());
		printf("Index: %u}\n", remove.index);
		break;
	}
	case CommandType::Doo:
	{
		printf("Project: %s, ", doo.project.c_str());
		printf("Tag: %s, ", doo.tag.c_str());
		printf("Index: %u, ", doo.index);
		printf("Status: [%c]}\n", doo.status);
		break;
	}
	case CommandType::Set:
	{
		printf("Index: %u, ", set.index);
		printf("Status: [%c]}\n", set.status);
		break;
	}
	default: printf("}\n"); break;	
	}	
}
