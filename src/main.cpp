
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <windows.h>
#include <ctime>
#include <iostream>
#include <string>

#include "command.h"

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

class Todo {
		
};

Command parseCommandType(char* str) {
	
	if (!strcasecmp(str, CommandStrings[CommandType::List]))   return Command(CommandType::List);
	if (!strcasecmp(str, CommandStrings[CommandType::Add]))    return Command(CommandType::Add);
	if (!strcasecmp(str, CommandStrings[CommandType::Remove])) return Command(CommandType::Remove);
	if (!strcasecmp(str, CommandStrings[CommandType::Doo]))    return Command(CommandType::Doo);
	if (!strcasecmp(str, CommandStrings[CommandType::Tidy]))   return Command(CommandType::Tidy);

	throw std::invalid_argument("Invalid command type!");
}

Command parseCommand(int argc, char** argv) {
	if (argc < 2)  throw std::invalid_argument("Not enough arguments!");
	
	Command command = parseCommandType(argv[1]);
	
	if (argc > 2 && command.type() == CommandType::List) {
		command.list.project = std::string(argv[2]);
	}

	return command;
}

void usage() {
	printf("usage: todo <command> [options]\n");
}

int main(int argc, char** argv) {
	
	setupConsole();
	
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
