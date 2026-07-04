#ifndef TODO_COMMAND_H
#define TODO_COMMAND_H

#include <cstdint>
#include <istream>
#include <ostream>
#include <set>
#include <string>

#define DEBUG_PRINT 0

class Todo;

enum CommandType {
	List,
	Add,
	Remove,
	Doo,
	Set,
	Undo,
	Tidy,
	Restore,
	None,
	CommandTypeSize
};

extern const char* CommandStrings[CommandType::CommandTypeSize];

inline std::ostream &operator<<(std::ostream &os, CommandType x) {
  os << (int)x;
  return os;
}

inline std::istream &operator>>(std::istream &is, CommandType &x) {
  int tmp;
  is >> tmp;
  x = CommandType(tmp);
  return is;
}

enum TaskType {
	Normal,
	Urgent,
	HighPriority,
	LowPriority,
	Completed,
	Suspended,
	Terminated,
	TaskTypeSize
};

extern const char* TaskColours[TaskType::TaskTypeSize];

inline std::ostream &operator<<(std::ostream &os, TaskType x) {
  os << (int)x;
  return os;
}

inline std::istream &operator>>(std::istream &is, TaskType &x) {
  int tmp;
  is >> tmp;
  x = TaskType(tmp);
  return is;
}

enum ListMode {
	Tasks,
	Projects,
	Tags
};

inline std::ostream &operator<<(std::ostream &os, ListMode x) {
  os << (int)x;
  return os;
}

inline std::istream &operator>>(std::istream &is, ListMode &x) {
  int tmp;
  is >> tmp;
  x = ListMode(tmp);
  return is;
}

TaskType parseTaskType(const std::string &line);

struct ListCommand {
	ListMode mode = ListMode::Tasks;
	std::string project;
	std::set<std::string> tags;
	std::set<char> statuses;
};

struct AddCommand {
	std::string project;
	std::string tag;
	std::string task;
	uint32_t index = 0;
};

struct RemoveCommand {
	std::string project;
	std::string tag;
	uint32_t index = 0;
};

struct DoCommand {
	std::string project;
	std::string tag;
	uint32_t index = 0;
	char status = 'x';
};

struct SetCommand {
	uint32_t index = 0;
	char status = 'x';
};

struct Command {
	Command() = default;
	explicit Command(CommandType commandType) : ct(commandType) {}
	
	CommandType type() const { return ct; }
	
	bool shouldUpdateHistory() const;
	void serialise(std::ostream &os) const;
	void deserialise(std::istream &is);
	void print() const;

	ListCommand list;
	AddCommand add;
	RemoveCommand remove;
	DoCommand doo;
	SetCommand set;

private:
	CommandType ct = CommandType::None;
};

Command inverseCommand(const Todo &todo, const Command &command);
void executeCommand(Todo &todo, Command &command);

#endif
