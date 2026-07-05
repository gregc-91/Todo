#ifndef TODO_COMMAND_H
#define TODO_COMMAND_H

#include <cstdint>
#include <set>
#include <string>

enum CommandType {
	List,
	Add,
	Remove,
	Doo,
	Set,
	Undo,
	Tidy,
	None
};

enum ListMode {
	Tasks,
	Projects,
	Tags
};

struct ListCommand {
	ListMode mode = Tasks;
	std::string project;
	std::set<std::string> tags;
	std::set<char> statuses;
};

struct AddCommand {
	std::string project;
	std::string tag;
	std::string task;
	uint32_t parentIndex = 0;
	bool hasParent = false;
};

struct RemoveCommand {
	uint32_t index = 0;
};

struct DoCommand {
	uint32_t index = 0;
	char status = 'x';
	bool tree = false;
};

struct SetCommand {
	uint32_t index = 0;
	char status = 'x';
	bool tree = false;
};

class Command {
public:
	Command() = default;
	explicit Command(CommandType commandType) : commandType(commandType) {}

	CommandType type() const { return commandType; }
	bool changesTodo() const;

	ListCommand list;
	AddCommand add;
	RemoveCommand remove;
	DoCommand doo;
	SetCommand set;

private:
	CommandType commandType = None;
};

#endif
