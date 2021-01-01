
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <windows.h>
#include <ctime>
#include <iostream>
#include <string>

#include "command.h"
#include "todo.h"

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING  0x0004
#endif

static HANDLE stdoutHandle;
static DWORD outModeInit;

void setupConsole(void) {
	DWORD outMode = 0;
	stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	if(stdoutHandle == INVALID_HANDLE_VALUE) {
		exit(GetLastError());
	}
	
	if(!GetConsoleMode(stdoutHandle, &outMode)) {
		exit(GetLastError());
	}

	outModeInit = outMode;
	
    // Enable ANSI escape codes
	outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

	if(!SetConsoleMode(stdoutHandle, outMode)) {
		exit(GetLastError());
	}	
}

void restoreConsole(void) {
    // Reset colors
    printf("\x1b[0m");	
	
    // Reset console mode
	if(!SetConsoleMode(stdoutHandle, outModeInit)) {
		exit(GetLastError());
	}
}

class Task {
	char status;
	std::string project;
	std::string tag;
	std::string text;
};

Command parseCommandType(char* str) {
	
	if (!strcasecmp(str, CommandStrings[CommandType::List]))   return Command(CommandType::List);
	if (!strcasecmp(str, CommandStrings[CommandType::Add]))    return Command(CommandType::Add);
	if (!strcasecmp(str, CommandStrings[CommandType::Remove])) return Command(CommandType::Remove);
	if (!strcasecmp(str, CommandStrings[CommandType::Doo]))    return Command(CommandType::Doo);
	if (!strcasecmp(str, CommandStrings[CommandType::Tidy]))   return Command(CommandType::Tidy);

	throw std::invalid_argument("Invalid command type!");
}

void parseListCommand(int argc, char** argv, Command &command) {
	
	for (int i = 0; i < argc; i++) {
		
		switch (argv[i][0]) {
		
		case '#':
			if (argv[i][1] != '\0') {
				if (command.list.project != "") throw std::runtime_error("Duplicate project name");
				command.list.project = std::string(argv[i]+1);
			} else {
				command.list.mode = ListMode::Projects;
			}
			break;
		case '@':
			if (argv[i][1] != '\0') {
				if (command.list.tag != "") throw std::runtime_error("Duplicate tag");
				command.list.tag = std::string(argv[i]+1);
			} else {
				command.list.mode = ListMode::Tags;
			}
			break;
		case '[':
			command.list.status = argv[i][1];
			if (argv[i][1] == '\0') throw std::runtime_error("Invalid status");
			if (argv[i][2] != ']') throw std::runtime_error("Invalid status");
			break;
		default:
			throw std::runtime_error("Unknown list subcommand");
		}
	}
}

void parseAddCommand(int argc, char** argv, Command &command) {
	
	for (int i = 0; i < argc; i++) {
		
		switch (argv[i][0]) {
			
		case '#':
			if (argv[i][1] == '\0') throw std::runtime_error("Missing project name");
			command.add.project = std::string(argv[i]+1);
			break;
		case '@':
			if (argv[i][1] == '\0') throw std::runtime_error("Missing tag name");
			command.add.tag = std::string(argv[i]+1);
			break;
		default:
			if (!command.add.task.empty()) throw std::runtime_error("Duplicate task name");
			command.add.task = std::string(argv[i]);
			break;
		}
	}
	
	if (command.add.task.empty())
		throw std::runtime_error("Missing line to add");
}

void parseRemoveCommand(int argc, char** argv, Command &command) {
	
	if (argc != 1)
		throw std::runtime_error("Invalid number of arguments");
	
	char *endptr;
	long line = strtol(argv[0], &endptr, 0);
	if (endptr == argv[0])
		throw std::runtime_error("Unable to parse line number");
	
	command.remove.index = line;
}

void parseDooCommand(int argc, char** argv, Command &command) {
	
	if (argc != 1)
		throw std::runtime_error("Invalid number of arguments");
	
	char *endptr;
	long line = strtol(argv[0], &endptr, 0);
	if (endptr == argv[0])
		throw std::runtime_error("Unable to parse line number");
	
	command.doo.index = line;
}

Command parseCommand(int argc, char** argv) {
	if (argc < 2)  throw std::invalid_argument("Not enough arguments!");
	
	Command command = parseCommandType(argv[1]);
	
	if (command.type() == CommandType::List) {
		parseListCommand(argc-2, argv+2, command);
	}
	if (command.type() == CommandType::Add) {
		parseAddCommand(argc-2, argv+2, command);
	}
	if (command.type() == CommandType::Remove) {
		parseRemoveCommand(argc-2, argv+2, command);
	}
	if (command.type() == CommandType::Doo) {
		parseDooCommand(argc-2, argv+2, command);
	}

	return command;
}

void usage() {
	printf("usage: todo [-h | --help] <command> [<subcommand...>]                                            \n");
	printf("         <command> :                                                                             \n");
	printf("           list                                 : lists all the tasks...   in a file             \n");
    printf("                    #<Project>                                               in a project        \n");
    printf("                    @<Tag>                                                   with tag            \n");
    printf("                    [<Status>]                                               with status         \n");
    printf("           list #                               : list all the projects... in a file             \n");
    printf("                    @<Tag>                                                   with tag            \n");
    printf("                    [<Status>]                                               with status         \n");
    printf("           list @                               : list all the tags...     in a file             \n");
    printf("                    #<Project>                                               in a project        \n");
    printf("                    [<Status>]                                               with status         \n");
	printf("                                                                                                 \n");
	printf("           add      [#Project] [@Tag] 'Task'    : adds a task with optional project and tag      \n");
	printf("                                                                                                 \n");
	printf("           remove   <line>                      : removes a task from the given line             \n");
	printf("           	                                                                                     \n");
	printf("           do       <line>                      : marks task on given line as done               \n");
	printf("           	                                                                                     \n");
	printf("           undo                                 : un-does the last action                        \n");
	printf("           	                                                                                     \n");
	printf("           tidy                                 : Formats the todo.txt file:                     \n");
	printf("           		                                    - Removes excess lines                       \n");
	printf("           		                                    - Fixes indentation and whitespace           \n");
	printf("           		                                    - Checks for syntax errors                   \n");
	printf("           		                                    - Moves done and suspended tasks to done.txt \n");
}

int main(int argc, char** argv) {
	
	setupConsole();
	
	if (argc > 1 && strncmp(argv[1], "-h", 2) == 0) {
		usage();
		exit(0);
	}
	
	
	Command command;
	
	try {
		command = parseCommand(argc, argv);
	} catch(std::invalid_argument &e) {
		std::cout << "Parse error: " << e.what() << std::endl;
		exit(-1);
	}
	
	try {
		executeCommand(command);
	} catch(...) {
		//Todo: handle execution execeptions
		std::cout << "Execution error" << std::endl;
		exit(-1);
	}
	
	std::cout << "Success!" << std::endl;
}
