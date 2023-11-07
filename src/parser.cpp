#include "parser.h"
#include "todo.h"

void validateStatus(char status) {
    if (strchr("-!^vx~.", status) == NULL) {
        printf("Error: invalid status\n");
        exit(0);
    }
}

Command parseCommandType(char* str)
{
    if (!strcasecmp(str, CommandStrings[CommandType::List])) return Command(CommandType::List);
    if (!strcasecmp(str, CommandStrings[CommandType::Add])) return Command(CommandType::Add);
    if (!strcasecmp(str, CommandStrings[CommandType::Remove])) return Command(CommandType::Remove);
    if (!strcasecmp(str, CommandStrings[CommandType::Doo])) return Command(CommandType::Doo);
    if (!strcasecmp(str, CommandStrings[CommandType::Set])) return Command(CommandType::Set);
    if (!strcasecmp(str, CommandStrings[CommandType::Undo])) return Command(CommandType::Undo);
    if (!strcasecmp(str, CommandStrings[CommandType::Tidy])) return Command(CommandType::Tidy);

    throw std::invalid_argument("Invalid command type!");
}

void parseListCommand(int argc, char** argv, Command& command)
{
    for (int i = 0; i < argc; i++) {
        switch (argv[i][0]) {
            case PROJECT_CHAR:
                if (argv[i][1] != '\0') {
                    if (command.list.project != "")
                        throw std::runtime_error("Duplicate project name");
                    command.list.project = std::string(argv[i] + 1);
                } else {
                    command.list.mode = ListMode::Projects;
                }
                break;
            case TAG_CHAR:
                if (argv[i][1] != '\0') {
                    command.list.tags.insert(std::string(argv[i] + 1));
                } else {
                    command.list.mode = ListMode::Tags;
                }
                break;
            case '[':
                {
                    if (argv[i][1] == '\0') throw std::runtime_error("Invalid status");
                    int j = 1;
                    while (argv[i][j] != '\0' && argv[i][j] != ']') {
                        validateStatus(argv[i][j]);
                        command.list.statuses.insert(argv[i][j++]);
                    }
                }
                break;
            default:
                throw std::runtime_error("Unknown list subcommand");
        }
    }
}

void parseAddCommand(int argc, char** argv, Command& command)
{
    for (int i = 0; i < argc; i++) {
        switch (argv[i][0]) {
            case PROJECT_CHAR:
                if (argv[i][1] == '\0') throw std::runtime_error("Missing project name");
                command.add.project = std::string(argv[i] + 1);
                break;
            case TAG_CHAR:
                if (argv[i][1] == '\0') throw std::runtime_error("Missing tag name");
                command.add.tag = std::string(argv[i] + 1);
                break;
            default:
                if (!command.add.task.empty()) throw std::runtime_error("Duplicate task name");
                command.add.task = std::string(argv[i]);
                break;
        }
    }

    if (command.add.task.empty()) throw std::runtime_error("Missing line to add");
}

void parseRemoveCommand(int argc, char** argv, Command& command)
{
    if (argc != 1) throw std::runtime_error("Invalid number of arguments");

    char* endptr;
    long line = strtol(argv[0], &endptr, 0);
    if (endptr == argv[0]) throw std::runtime_error("Unable to parse line number");

    command.remove.index = line;
}

void parseDooCommand(int argc, char** argv, Command& command)
{
    if (argc != 1) throw std::runtime_error("Invalid number of arguments");

    char* endptr;
    long line = strtol(argv[0], &endptr, 0);
    if (endptr == argv[0]) throw std::runtime_error("Unable to parse line number");

    command.doo.index = line;
}

void parseSetCommand(int argc, char** argv, Command& command)
{
    if (argc != 2) throw std::runtime_error("Invalid number of arguments");

    char* endptr;
    char status = argv[0][0];
    long line = strtol(argv[1], &endptr, 0);
    if (endptr == argv[1]) throw std::runtime_error("Unable to parse line number");

    validateStatus(status);

    command.set.index = line;
    command.set.status = status;
}

Command parseCommand(int argc, char** argv)
{
    if (argc < 2) throw std::invalid_argument("Not enough arguments!");

    Command command = parseCommandType(argv[1]);

    if (command.type() == CommandType::List) {
        parseListCommand(argc - 2, argv + 2, command);
    }
    if (command.type() == CommandType::Add) {
        parseAddCommand(argc - 2, argv + 2, command);
    }
    if (command.type() == CommandType::Remove) {
        parseRemoveCommand(argc - 2, argv + 2, command);
    }
    if (command.type() == CommandType::Doo) {
        parseDooCommand(argc - 2, argv + 2, command);
    }
    if (command.type() == CommandType::Set) {
        parseSetCommand(argc - 2, argv + 2, command);
    }

    return command;
}