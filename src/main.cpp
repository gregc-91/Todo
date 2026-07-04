#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "command.h"
#include "console.h"
#include "parser.h"
#include "todo.h"

namespace {
void usage()
{
	std::cout
		<< "usage:\n"
		<< "  todo [-h | --help] <command> [<subcommand...>]\n"
		<< "    <command>:\n"
		<< "      list                             : lists all the tasks...   in a file\n"
		<< "              +<Project>                                          in a project\n"
		<< "              @<Tag>                                              with tag\n"
		<< "              [<Status...>]                                       with status\n"
		<< "      list +                           : lists all the projects... in a file\n"
		<< "              +<Project>                                          matching a project\n"
		<< "              @<Tag>                                              containing tag\n"
		<< "              [<Status...>]                                       containing status\n"
		<< "      list @                           : lists all the tags...     in a file\n"
		<< "              +<Project>                                          in a project\n"
		<< "              [<Status...>]                                       with status\n"
		<< "\n"
		<< "      add     [+Project] [@Tag] \"Task\" : adds a task with optional project and tag\n"
		<< "\n"
		<< "      remove  <line>                   : removes the given zero-based line\n"
		<< "\n"
		<< "      do      <line>                   : marks the task on the given line as done\n"
		<< "\n"
		<< "      set     <Status> <line>          : sets the status on the given line\n"
		<< "\n"
		<< "      undo                             : undoes the last action\n"
		<< "\n"
		<< "      tidy                             : normalizes blank lines between projects\n"
		<< "\n"
		<< "    <Status>: - active, ! urgent, ^ high, v low, x done,\n"
		<< "              ~ suspended, . terminated\n"
		<< "\n"
		<< "    Use +Project in commands; projects are stored as \"# Project\" in todo.txt.\n";
}
}

int main(int argc, char **argv)
{
	Console::setup();
	std::atexit(Console::restore);

	if (argc > 1 &&
		(std::strcmp(argv[1], "-h") == 0 ||
		 std::strcmp(argv[1], "--help") == 0)) {
		usage();
		return EXIT_SUCCESS;
	}

	try {
		Todo todo("todo.txt");
		Command command = parseCommand(argc, argv);
		Command inverse = inverseCommand(todo, command);

		command.print();
		inverse.print();
		executeCommand(todo, command);

		if (command.shouldUpdateHistory()) {
			std::ofstream historyStream("history.txt");
			if (!historyStream) {
				throw std::runtime_error(
					"unable to open command history for writing");
			}
			command.serialise(historyStream);
			inverse.serialise(historyStream);
			if (!historyStream) {
				throw std::runtime_error("failed while writing command history");
			}
		}
	} catch (const std::exception &error) {
		std::cerr << "Error: " << error.what() << '\n';
		return EXIT_FAILURE;
	} catch (...) {
		std::cerr << "Error: unexpected failure\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
