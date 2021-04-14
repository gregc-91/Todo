
#include "command.h"
#include "todo.h"

#include <strings.h>
#include <fstream>
#include <iostream>
#include <iomanip>

const char* CommandStrings[CommandType::CommandTypeSize] = {
	"List",
	"Add",
	"Remove",
	"Do",
	"Tidy",
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

std::string trimLeadingWhitespace(const std::string &str) {
	size_t start = str.find_first_not_of(whitespace);
	return start == std::string::npos ? "" : str.substr(start);
}

std::string createTaskString(const char status, std::string line, std::string tag) {
	return "[" + std::string(1, status) + "] " + line + (tag.empty() ? "" : (" @" + tag));
}

bool isProject(const std::string &str, const std::string &project) {
	std::string line = trimLeadingWhitespace(str);
	if (line.front() != '#') return false;
	
	return project == line.substr(1);
}

bool containsTag(const std::string &str, const std::string &tag) {
	std::stringstream stream(str);
	std::string intermediate;
	while(getline(stream, intermediate, ' ')) {
		if (strcasecmp(intermediate.c_str(), ("@" + tag).c_str()) == 0)
			return true;
	}
	return false;
}

bool hasStatus(const std::string &str, const char status) {
	if (str.size() >= 3 && str[0] == '[' && str[1] == status && str[2] == ']') {
		return true;
	}
	return false;
}

unsigned findLastOfProject(const Todo &todo, const std::string &project) {
	
	if (project.empty())
		return todo.lines.size();
	
	bool matchProject = false;
	unsigned lastNonEmpty = 0;
	for (unsigned i = 0; i < todo.lines.size(); i++) {
		std::string line = trimLeadingWhitespace(todo.lines[i]);
		if (line[0] == '#') {
			if (isProject(todo.lines[i], project)) matchProject = true;
			else if (matchProject) return lastNonEmpty+1;
		}
		if (!line.empty()) lastNonEmpty = i;
	}
	
	return todo.lines.size();
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

// Todo: convert this to using the Todo class
static void executeListCommand(const Command command) {
	printf("Executing list command.\n");

	std::ifstream file("todo.txt");
	
	bool filterProject = command.list.project != "";
	bool filterTag     = command.list.tag     != "";
	bool filterStatus  = command.list.status  != '_';

	unsigned lineNo = 0;
	std::string projectName = "";
	for(std::string line; getline(file, line); lineNo++) {

		line = trimLeadingWhitespace(line);
		bool isProject = line.front() == '#';
		bool isTask    = line.front() == '[';
		
		if (isProject) {
			std::stringstream lineStream(trimLeadingWhitespace(line.substr(1))); 
			std::getline(lineStream, projectName, ' ');
		}
		
		if (command.list.mode == ListMode::Projects && !isProject) {
			continue;
		}
		
		if (filterProject && strcasecmp(command.list.project.c_str(), projectName.c_str())) {
			continue;
		}
		
		if (filterTag && !containsTag(line, command.list.tag)) {
			continue;
		}
		
		if (filterStatus && !hasStatus(line, command.list.status)) {
			continue;
		}
		
		std::cout << Colour::BrightBlack << std::setw(4) << lineNo << ":  " << Colour::Reset;
		
		if (isProject) {
			std::cout << Colour::BrightWhite;
		} else if (isTask) {
			TaskType taskType = parseTaskType(line);
			
			std::cout << TaskColours[taskType];
			
			//line = line.subtr(3);
		}
		
		std::cout << line << std::endl;
		
		std::cout << Colour::Reset;
	}
}

static void executeAddCommand(Todo& todo, Command &command) {
	printf("Executing add command.\n");
	
	std::string taskString = createTaskString('-', command.add.task, command.add.tag);
	
	command.add.index = findLastOfProject(todo, command.add.project);
	
	todo.addLine(command.add.index, taskString);
	todo.commit();
}

static void executeRemoveCommand(Todo& todo, const Command command) {
	printf("Executing remove command.\n");
	
	todo.removeLine(command.remove.index);
	todo.commit();
}

static void executeDooCommand(Todo& todo, const Command command) {
	printf("Executing do command.\n");
	
	todo.setStatus(command.doo.index, 'x');
	todo.commit();
}

static void executeTidyCommand(const Command /*command*/) {
	printf("Executing tidy command.\n");
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
	case CommandType::Tidy:
		{
			executeTidyCommand(/*todo,*/ command);
		} break;
	default: break;
	}
}

// Todo: check these
std::string lineToProject(const Todo &todo, uint32_t index)
{
	for (int i = index; i >= 0; i--) {
		if (todo.lines[i].front() == '#') {
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
	
	printf("Status for line %d: %c\n", index, todo.lines[index][start]);
	return todo.lines[index][start];
}

Command inverseCommand(const Todo &todo, const Command command)
{
	// List : None
	// Add  : Remove
	// Doo  : Undo
	// Tidy : None
	
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
	default: break;
	}
	
	return Command();
}

void serialise_string(std::ostream &os, std::string &str)
{
	os << str.length() << ',';
	if (str.length()) {
		os << str;
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
		is >> str[i];
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
		serialise_string(os, list.tag);
		os << list.status << ',';
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
		os << list.status << ',';
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
		new (&list.tag) std::string();
		deserialise_string(is, list.tag);
		is >> list.status >> tmp;
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
	case CommandType::Tidy:
	{
		break;
	}
	default:	
		throw std::runtime_error("Unexpected command type in command history deserialisation");
	}
}

void Command::print()
{
	printf("{Type: %s, ", CommandStrings[ct]);
	
	switch (ct) {
	case CommandType::List:
	{
		printf("Mode: %d, ", list.mode);
		printf("Project: %s, ", list.project.c_str());
		printf("Tag: %s, ", list.tag.c_str());
		printf("Status: [%c]}\n", list.status);
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
	case CommandType::Tidy:
	{
		printf("}\n");
		break;
	}
	default: printf("}\n"); break;	
	}	
}