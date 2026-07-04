
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "command.h"
#include "todo.h"
#include "parser.h"
#include "console.h"

void usage()
{
    printf("usage: \n");
    printf("  todo [-h | --help] <command> [<subcommand...>]                                  \n");
    printf("    <command> :                                                                   \n");
    printf("      list                             : lists all the tasks...   in a file       \n");
    printf("              %c<Project>                                          in a project   \n", PROJECT_CHAR);
    printf("              %c<Tag>                                              with tag       \n", TAG_CHAR);
    printf("              [<Status>]                                          with status     \n");
    printf("      list %c                           : list all the projects... in a file      \n", PROJECT_CHAR);
    printf("              %c<Tag>                                              with tag       \n", TAG_CHAR);
    printf("              [<Status>]                                          with status     \n");
    printf("      list %c                           : list all the tags...     in a file      \n", TAG_CHAR);
    printf("              %c<Project>                                          in a project   \n", PROJECT_CHAR);
    printf("              [<Status>]                                          with status     \n");
    printf("                                                                                  \n");
    printf("      add     [%cProject] [%cTag] 'Task' : adds a task with optional project and tag\n", PROJECT_CHAR, TAG_CHAR);
    printf("                                                                                  \n");
    printf("      remove  <line>                   : removes a task from the given line       \n");
    printf("                                                                                  \n");
    printf("      do      <line>                   : marks task on given line as done         \n");
    printf("                                                                                  \n");
    printf("      set     <Status> <line>          : set the status on the given line         \n");
    printf("                                                                                  \n");
    printf("      undo                             : un-does the last action                  \n");
    printf("                                                                                  \n");
    printf("      tidy                             : Formats the todo.txt file:               \n");
    printf("                                         - Removes excess lines                   \n");
    printf("                                         - Fixes indentation and whitespace       \n");
    printf("                                         - Checks for syntax errors               \n");
    printf("                                         - Moves non-active tasks to done.txt     \n");
}

int main(int argc, char** argv)
{
    Console::setup();
    std::atexit(Console::restore);

    if (argc > 1 &&
        (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        usage();
        exit(0);
    }

	Todo todo("todo.txt");
    Command command;
	Command inverse;
	
    try {
        command = parseCommand(argc, argv);
		inverse = inverseCommand(todo, command);
    } catch (std::invalid_argument& e) {
        std::cout << "Parse error: " << e.what() << std::endl;
        exit(-1);
    } catch (std::runtime_error& e) {
        std::cout << "Error: " << e.what() << std::endl;
        exit(-1);
    }

	if (DEBUG_PRINT) printf("Command:\n  ");
	command.print();
	if (DEBUG_PRINT) printf("Inverse:\n  ");
	inverse.print();

    try {
        executeCommand(todo, command);
		
		if (command.shouldUpdateHistory()) {
			// Update the history file so it can be undone
			std::ofstream history_stream("history.txt");
			command.serialise(history_stream);
			inverse.serialise(history_stream);
			history_stream.close();
		}
    } catch (...) {
        // Todo: handle execution execeptions
        std::cout << "Execution error" << std::endl;
        exit(-1);
    }

    if (DEBUG_PRINT) {std::cout << "Success!" << std::endl;}
}
