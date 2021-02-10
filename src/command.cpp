
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
	std::string line = trimLeadingWhitespace(project);
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
	for (unsigned i = 0; i < todo.lines.size(); i++) {
		std::string line = trimLeadingWhitespace(todo.lines[i]);
		if (line[0] == '#') {
			if (isProject(todo.lines[i], project)) matchProject = true;
			else if (matchProject) return i;
		}
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

static void executeAddCommand(const Command command) {
	printf("Executing add command.\n");
	
	Todo todo("todo.txt");
	std::string taskString = createTaskString('-', command.add.task, command.add.tag);
	
	unsigned lineNo = findLastOfProject(todo, command.add.project);
	
	todo.addLine(lineNo, taskString);
	todo.commit();
}

static void executeRemoveCommand(const Command command) {
	printf("Executing remove command.\n");
	
	Todo todo("todo.txt");
	todo.removeLine(command.remove.index);
	todo.commit();
}

static void executeDooCommand(const Command command) {
	printf("Executing do command.\n");
	
	Todo todo("todo.txt");
	todo.setStatus(command.doo.index, 'x');
	todo.commit();
}

static void executeTidyCommand(const Command /*command*/) {
	printf("Executing tidy command.\n");
}

void executeCommand(const Command command) {
	
	switch (command.type()) {
	case CommandType::List:
		{
			executeListCommand(command);
		} break;
	case CommandType::Add:
		{
			executeAddCommand(command);
		} break;
	case CommandType::Remove:
		{
			executeRemoveCommand(command);
		} break;
	case CommandType::Doo:
		{
			executeDooCommand(command);
		} break;
	case CommandType::Tidy:
		{
			executeTidyCommand(command);
		} break;
	default: break;
	}
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