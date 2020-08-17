
#include "command.h"

#include <fstream>
#include <iostream>
#include <iomanip>

namespace Colour {
	static const char* Black  = "\u001b[30;1m";
	static const char* Red    = "\u001b[31;1m";
	static const char* Green  = "\u001b[32;1m";
	static const char* Yellow = "\u001b[33;1m";
	static const char* Blue   = "\u001b[34;1m";
	static const char* Pink   = "\u001b[35;1m";
	static const char* Cyan   = "\u001b[36;1m";
	static const char* White  = "\u001b[37;1m";
	static const char* Reset  = "\u001b[0m";
}

const char* CommandStrings[CommandType::CommandTypeSize] = {
	"List",
	"Add",
	"Remove",
	"Do",
	"Tidy",
	"None"
};

static void executeListCommand(const Command /*command*/) {
	printf("Executing list command.\n");
	
	const std::string whitespace = " \t\f\v\n\r";
	
	std::ifstream file("todo.txt");

	unsigned lineNo = 0;
	for(std::string line; getline(file, line); lineNo++) {

		int start = line.find_first_not_of(whitespace);
		bool isProject = line[start] == '#';
		
		 std::cout << Colour::Black << std::setw(4) << lineNo << ":  " << Colour::Reset;
		
		if (isProject) {
			std::cout << Colour::White;
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