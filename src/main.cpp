#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>

#include "command.h"
#include "console.h"
#include "history.h"
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
		<< "              --under <line> [@Tag] \"Task\"                        as a subtask\n"
		<< "\n"
		<< "      remove  <line>                   : removes a task and all its subtasks\n"
		<< "\n"
		<< "      do      <line>                   : marks the task on the given line as done\n"
		<< "              <line> --tree                                      including subtasks\n"
		<< "\n"
		<< "      set     <Status> <line>          : sets the status on the given line\n"
		<< "              <Status> <line> --tree                              including subtasks\n"
		<< "\n"
		<< "      undo                             : undoes the last action\n"
		<< "\n"
		<< "      tidy                             : validates hierarchy and normalizes blank lines\n"
		<< "\n"
		<< "    <Status>: - active, ! urgent, ^ high, v low, x done,\n"
		<< "              ~ suspended, . terminated\n"
		<< "\n"
		<< "    Use +Project in commands; projects are stored as \"# Project\" in todo.txt.\n"
		<< "    Subtasks use the same status notation, indented by two spaces per level.\n";
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

		if (command.shouldUpdateHistory()) {
			History history("history.txt");
			executeCommand(todo, command);
			history.push(command, inverse);
			try {
				history.commit();
			} catch (const std::exception &historyError) {
				try {
					todo.lines = inverse.restore.lines;
					todo.commit();
				} catch (const std::exception &rollbackError) {
					throw std::runtime_error(
						std::string("history update failed: ") +
						historyError.what() + "; rollback failed: " +
						rollbackError.what());
				}
				throw std::runtime_error(
					std::string("history update failed; change was rolled back: ") +
					historyError.what());
			}
		} else {
			executeCommand(todo, command);
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
