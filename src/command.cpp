
#include "command.h"

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

bool containsTag(const std::string &str, const std::string &tag) {
	std::stringstream stream(str);
	std::string intermediate;
	while(getline(stream, intermediate, ' ')) {
		if (strcasecmp(intermediate.c_str(), ("@" + tag).c_str()) == 0)
			return true;
	}
	return false;
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

static void executeAddCommand(const Command /*command*/) {
	printf("Executing add command.\n");
}

static void executeRemoveCommand(const Command /*command*/) {
	printf("Executing remove command.\n");
}

static void executeDooCommand(const Command /*command*/) {
	printf("Executing do command.\n");
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