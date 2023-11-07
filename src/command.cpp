
#include "command.h"
#include "todo.h"
#include "colour.h"

#include <strings.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <list>
#include <set>
#include <algorithm>

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

std::string createTaskString(const char status, std::string line, std::string tag) {
	return "[" + std::string(1, status) + "] " + line + (tag.empty() ? "" : (std::string(1, TAG_CHAR) + tag));
}

bool isAnyProject(const std::string &str) {
	std::string trimmedLine = trimLeadingWhitespace(str);
	bool isProject = trimmedLine.front() == PROJECT_CHAR;
	return isProject;
}

bool isProject(const std::string &str, const std::string &project) {
	std::string line = trimLeadingWhitespace(str);
	if (line.front() != PROJECT_CHAR) return false;
	
	return project == line.substr(1);
}

bool containsTag(const std::string &str, const std::string &tag) {
	std::stringstream stream(str);
	std::string intermediate;
	while(getline(stream, intermediate, ' ')) {
		if (strcasecmp(intermediate.c_str(), (std::string(1, TAG_CHAR) + tag).c_str()) == 0)
			return true;
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
		if (line[0] == PROJECT_CHAR) {
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

TaskType parseTaskType(std::string &line) {
	assert(line.size() >= 3);
	assert(line[0] == '[');
	assert(line[2] == ']');
	assert(line[3] == ' ');
	
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
			if (intermediate.length() > 1) tags.insert(intermediate);
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
	std::vector<Line> lines;
};

void printLine(std::string line, uint32_t lineNo) {

	std::string trimmedLine = trimLeadingWhitespace(line);
	bool isProject = trimmedLine.front() == PROJECT_CHAR;
	bool isTask    = trimmedLine.front() == '[';

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

// Todo: convert this to using the Todo class
static void executeListCommand(const Command command) {
	if (DEBUG_PRINT) printf("Executing list command.\n");

	std::ifstream file("todo.txt");
	
	bool filterProject = command.list.project != "";
	bool filterTag     = !command.list.tags.empty();
	bool filterStatus  = !command.list.statuses.empty();
	std::set<std::string> tagsInFile;
	std::vector<Project> projects;

	unsigned lineNo = 0;
	std::string projectName = "";
	for(std::string line; getline(file, line); lineNo++) {

		std::string trimmedLine = trimLeadingWhitespace(line);
		bool isProject = trimmedLine.front() == PROJECT_CHAR;
		
		if (isProject) {
			std::stringstream lineStream(trimLeadingWhitespace(trimmedLine.substr(1))); 
			std::getline(lineStream, projectName, ' ');

			projects.push_back({trimmedLine.substr(1), lineNo, std::vector<Line>()});
		}
		
		if (command.list.mode == ListMode::Projects && !isProject) {
			continue;
		}

		if (command.list.mode == ListMode::Tags) {
			std::set<std::string> tagsInLine;
			tagsInFile.merge(getTags(trimmedLine));
			continue;
		}
		
		if (filterProject && strcasecmp(command.list.project.c_str(), projectName.c_str())) {
			continue;
		}
		
		if (filterTag && !containsTagFromSet(trimmedLine, command.list.tags)) {
			continue;
		}
		
		if (filterStatus && !hasStatusFromSet(trimmedLine, command.list.statuses)) {
			continue;
		}
		
		// Create a dummy project if we have yet to see one
		if (projects.empty()) {
			projects.push_back({std::string(""), lineNo, std::vector<Line>()});
		}
		projects.back().lines.push_back({line, lineNo});
	}

	// Remove empty projects
	projects.erase(std::remove_if(
			projects.begin(), projects.end(), [](const Project& p) { return p.lines.empty(); }
		), projects.end()
	);

	if (command.list.mode == ListMode::Tags) {
		for (auto tag : tagsInFile) {
			std::cout << "  " << tag << std::endl;
		}
	} else {
		bool printProjectName = 
			command.list.mode == ListMode::Projects ||
			(command.list.mode == ListMode::Tasks && (filterTag || filterStatus));

		size_t i = 0;
		for (auto &project : projects) {
			if (project.lines.empty()) 
				continue;

			if (printProjectName) {
				printLine(std::string("+") + project.name, project.no);
			}

			for (auto &line : project.lines) {
				printLine(line.line, line.no);
			}

			if (printProjectName && i < projects.size()-1) {
				std::cout << std::endl;
			}
			i++;
		}
	}
}

static void executeAddCommand(Todo& todo, Command &command) {
	if (DEBUG_PRINT) printf("Executing add command.\n");
	
	std::string taskString = createTaskString('-', command.add.task, command.add.tag);
	
	int index = findLastOfProject(todo, command.add.project);

	if (index == -1) { // Project doesn't exist, add it first
		// If the last last isn't blank, add one
		if (!std::all_of(todo.lines.back().begin(),todo.lines.back().end(),isspace)) {
			todo.addLine(todo.lines.size(), std::string(""));
		}
		todo.addLine(todo.lines.size(), std::string(1, PROJECT_CHAR) + command.add.project);
		index = todo.lines.size();
	}

	command.add.index = index;
	todo.addLine(command.add.index, taskString);
	todo.commit();
	todo.printLine(command.add.index);
}

static void executeRemoveCommand(Todo& todo, const Command command) {
	if (DEBUG_PRINT) printf("Executing remove command.\n");
	
	todo.printLine(command.remove.index);
	todo.removeLine(command.remove.index);
	todo.commit();
}

static void executeDooCommand(Todo& todo, const Command command) {
	if (DEBUG_PRINT) printf("Executing do command.\n");
	
	todo.setStatus(command.doo.index, command.doo.status);
	todo.commit();
	todo.printLine(command.doo.index);
}

static void executeSetCommand(Todo& todo, const Command command) {
	if (DEBUG_PRINT) printf("Executing set command.\n");
	
	todo.setStatus(command.set.index, command.set.status);
	todo.commit();
	todo.printLine(command.set.index);
}

static void executeUndoCommand(Todo& todo, const Command command) {
	if (DEBUG_PRINT) printf("Executing undo command.\n");
	
	Command previous;
	Command inverse;
	
	// Read the last command and inverse command from the history file
	try {
		std::ifstream history_stream("history.txt");
		previous.deserialise(history_stream);
		inverse.deserialise(history_stream);
	} catch (std::runtime_error& e) {
        std::cout << "Command history error: " << e.what() << std::endl;
        exit(-1);
    }
	
	// Print the last commands to check them
	previous.print();
	inverse.print();
	
	executeCommand(todo, inverse);
}

static void executeTidyCommand(Todo &todo, const Command /*command*/) {
	if (DEBUG_PRINT) printf("Executing tidy command.\n");

	todo.backup();

	for(size_t i = 1; i < todo.lines.size(); i++) {
		// Remove duplicate blank lines
		if (std::all_of(todo.lines[i-1].begin(),todo.lines[i-1].end(),isspace) && 
			std::all_of(todo.lines[i].begin(),todo.lines[i].end(),isspace)) {
			todo.removeLine(i--);
			continue;
		}

		// Remove blank lines which aren't immediately before a project
		if (std::all_of(todo.lines[i-1].begin(),todo.lines[i-1].end(),isspace) &&
			!isAnyProject(todo.lines[i])) {
			todo.removeLine(--i);
			continue;
		}
	}

	// Add a blank line before a project
	for(size_t i = 1; i < todo.lines.size(); i++) {
		if (!std::all_of(todo.lines[i-1].begin(),todo.lines[i-1].end(),isspace) &&
			isAnyProject(todo.lines[i])) {
			todo.addLine(i, "");
		}
	}

	todo.commit();
}
static void executeRestoreCommand(Todo &todo, const Command /*command*/) {
	todo.restore();
	todo.commit();
}

void executeCommand(Todo &todo, Command &command) {
	
	switch (command.type()) {
	case CommandType::List:
		{
			executeListCommand(/*todo, */command);
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

// Todo: check these
std::string lineToProject(const Todo &todo, uint32_t index)
{
	if (index >= todo.lines.size()) {
		throw std::runtime_error("Index out of bounds");
	}

	for (int i = index; i >= 0; i--) {
		if (todo.lines[i].front() == PROJECT_CHAR) {
			return trimLeadingWhitespace(todo.lines[i]).substr(1);
		}
	}
	return std::string("");
}

std::string lineToTag(const Todo &todo, uint32_t index)
{
	size_t pos = todo.lines[index].find_last_of(" ");
	if (pos != std::string::npos && pos+1 < todo.lines[index].length() && todo.lines[index][pos+1] == '@') {
		return todo.lines[index].substr(pos+2);
	}
	return "";
}

std::string lineToTask(const Todo &todo, uint32_t index)
{
	size_t start = todo.lines[index].find_first_of("[") + 3;
	
	if (start >= todo.lines[index].length()) {
		throw std::runtime_error("Failed to parse task string");
	}
	
	std::string line = todo.lines[index].substr(start);
	
	size_t pos = line.find_last_of(" ");
	if (pos != std::string::npos && pos+1 < todo.lines[index].length() && todo.lines[index][pos+1] == '@') {
		return line.substr(0, pos);
	}
	return line;
}

char lineToStatus(const Todo &todo, uint32_t index) 
{
	size_t start = todo.lines[index].find_first_of("[") + 1;
	
	if (start >= todo.lines[index].length()) {
		throw std::runtime_error("Failed to parse task string");
	}
	
	if (DEBUG_PRINT) printf("Status for line %d: %c\n", index, todo.lines[index][start]);
	return todo.lines[index][start];
}

Command inverseCommand(const Todo &todo, const Command command)
{
	// List : None
	// Add  : Remove
	// Doo  : Undo
	// Set  : Set
	// Tidy : Restore
	
	switch (command.type()) {
	case CommandType::Add:
		{
			Command inverse(CommandType::Remove);
			inverse.remove.project = command.add.project;
			inverse.remove.tag = command.add.tag;
			inverse.remove.index = findLastOfProject(todo, command.add.project);
			return inverse;
		} break;
	case CommandType::Remove:
		{
			Command inverse(CommandType::Add);
			inverse.add.project = lineToProject(todo, command.remove.index);
			inverse.add.tag = lineToTag(todo, command.remove.index);
			inverse.add.task = lineToTask(todo, command.remove.index);
			inverse.add.index = command.remove.index;
			return inverse;
		} break;
	case CommandType::Doo:
		{
			Command inverse(CommandType::Doo);
			inverse.doo.project = command.doo.project;
			inverse.doo.tag = command.doo.tag;
			inverse.doo.index = command.doo.index;
			inverse.doo.status = lineToStatus(todo, command.doo.index);
			return inverse;
		} break;
	case CommandType::Set:
		{
			Command inverse(CommandType::Set);
			inverse.set.index = command.set.index;
			inverse.set.status = lineToStatus(todo, command.set.index);
			return inverse;
		} break;
	case CommandType::Tidy:
		{
			Command inverse(CommandType::Restore);
			return inverse;
		} break;
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

void serialise_set(std::ostream &os, std::set<std::string> &set)
{
	os << set.size() << ',';
	for (auto &str : set) {
		serialise_string(os, str);
	}
}

void serialise_set(std::ostream &os, std::set<char> &set)
{
	os << set.size() << ',';
	for (auto &chr : set) {
		os << chr;
	}
	os << ',';
}

void deserialise_string(std::istream &is, std::string &str)
{
	size_t length;
	char junk;
	is >> length >> junk;
	if (length) str.resize(length);
	for (unsigned i = 0; i < length; i++) {
		is >> std::noskipws >> str[i];
	}
	is >> junk;
}

void deserialise_set(std::istream &is, std::set<std::string> &set)
{
	size_t length;
	char junk;
	is >> length >> junk;
	for (unsigned i = 0; i < length; i++) {
		std::string str;
		deserialise_string(is, str);
		set.insert(str);
	}
}

void deserialise_set(std::istream &is, std::set<char> &set)
{
	size_t length;
	char junk;
	is >> length >> junk;
	for (unsigned i = 0; i < length; i++) {
		is >> std::noskipws >> junk;
		set.insert(junk);
	}
	is >> junk;
}

void Command::serialise(std::ostream &os) 
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
	default: break;	
	}
}

void Command::deserialise(std::istream &is)
{
	char tmp;
	is >> ct >> tmp;
	
	switch (ct) {
	case CommandType::List:
	{
		is >> list.mode >> tmp;
		new (&list.project) std::string();
		deserialise_string(is, list.project);
		new (&list.tags) std::set<std::string>();
		deserialise_set(is, list.tags);
		new (&list.statuses) std::set<char>();
		deserialise_set(is, list.statuses);
		break;
	}
	case CommandType::Add:
	{
		new (&add.project) std::string();
		deserialise_string(is, add.project);
		new (&add.tag) std::string();
		deserialise_string(is, add.tag);
		new (&add.task) std::string();
		deserialise_string(is, add.task);
		is >> add.index >> tmp;
		break;
	}
	case CommandType::Remove:
	{
		new (&remove.project) std::string();
		deserialise_string(is, remove.project);
		new (&remove.tag) std::string();
		deserialise_string(is, remove.tag);
		is >> remove.index >> tmp;
		break;
	}
	case CommandType::Doo:
	{
		new (&doo.project) std::string();
		deserialise_string(is, doo.project);
		new (&doo.tag) std::string();
		deserialise_string(is, doo.tag);
		is >> doo.index >> tmp;
		is >> doo.status >> tmp;
		break;
	}
	case CommandType::Set:
	{
		is >> set.index >> tmp;
		is >> set.status >> tmp;
		break;
	}
	case CommandType::Tidy:
	{
		break;
	}
	case CommandType::Restore:
	{
		break;
	}
	default:	
		throw std::runtime_error("Unexpected command type in command history deserialisation");
	}
}

bool Command::shouldUpdateHistory()
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

void Command::print()
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
		printf("Index: %d}\n", add.index);
		break;
	}
	case CommandType::Remove:
	{
		printf("Project: %s, ", remove.project.c_str());
		printf("Tag: %s, ", remove.tag.c_str());
		printf("Index: %d}\n", remove.index);
		break;
	}
	case CommandType::Doo:
	{
		printf("Project: %s, ", doo.project.c_str());
		printf("Tag: %s, ", doo.tag.c_str());
		printf("Index: %d, ", doo.index);
		printf("Status: [%c]}\n", doo.status);
		break;
	}
	case CommandType::Set:
	{
		printf("Index: %d, ", doo.index);
		printf("Status: [%c]}\n", doo.status);
		break;
	}
	default: printf("}\n"); break;	
	}	
}