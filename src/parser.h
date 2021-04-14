#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "command.h"

Command parseCommandType(char* str);

void parseListCommand(int argc, char** argv, Command& command);

void parseAddCommand(int argc, char** argv, Command& command);

void parseRemoveCommand(int argc, char** argv, Command& command);

void parseDooCommand(int argc, char** argv, Command& command);

Command parseCommand(int argc, char** argv);