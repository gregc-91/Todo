#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <assert.h>
#include <string>
#include <ostream>
#include <istream>
#include <cstdint>
#include <set>

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

TaskType parseTaskType(std::string &line);

struct Command {
	Command() : ct(CommandType::None) {
	}
	
	Command(CommandType commandType) {
		ct = commandType;
		switch (ct) {
		case CommandType::List: 
			list.mode = ListMode::Tasks;
			new (&list.statuses)std::set<char>();
			new (&list.project) std::string();
			new (&list.tags)    std::set<std::string>();
			break;
		case CommandType::Add:
			add.index = 0;
			new (&add.project) std::string();
			new (&add.tag)     std::string();
			new (&add.task)    std::string();
			break;
		case CommandType::Remove:
			remove.index = 0;
			new (&remove.project) std::string();
			new (&remove.tag)     std::string();
			break;
		case CommandType::Doo:
			doo.index = 0;
			doo.status = 'x';
			new (&doo.project) std::string();
			new (&doo.tag)     std::string();
			break;
		case CommandType::Set:
			set.index = 0;
			set.status = 'x';
			break;
		default: break;
		}		
	}
	
	Command(const Command& other) {
		ct = other.type();
		switch (ct) {
		case CommandType::List: 
			list.mode = other.list.mode;
			new (&list.statuses)std::set(other.list.statuses);
			new (&list.project) std::string(other.list.project);
			new (&list.tags)    std::set(other.list.tags);
			break;
		case CommandType::Add:
			add.index = other.add.index;
			new (&add.project) std::string(other.add.project);
			new (&add.tag)     std::string(other.add.tag);
			new (&add.task)    std::string(other.add.task);
			break;
		case CommandType::Remove:
			remove.index = other.remove.index;
			new (&remove.project) std::string(other.remove.project);
			new (&remove.tag)     std::string(other.remove.tag);
			break;
		case CommandType::Doo:
			doo.index = other.doo.index;
			doo.status = other.doo.status;
			new (&doo.project) std::string(other.doo.project);
			new (&doo.tag)     std::string(other.doo.tag);
			break;
		case CommandType::Set:
			set.index = other.set.index;
			set.status = other.set.status;
		default: break;
		}		
	}
	
	Command& operator=(const Command& other) {
		assert(ct == CommandType::None);
		ct = other.type();
		switch (ct) {
		case CommandType::List: 
			list.mode = other.list.mode;
			new (&list.statuses) std::set(other.list.statuses);
			new (&list.project) std::string(other.list.project);
			new (&list.tags)    std::set(other.list.tags);
			break;
		case CommandType::Add:
			add.index = other.add.index;
			new (&add.project) std::string(other.add.project);
			new (&add.tag)     std::string(other.add.tag);
			new (&add.task)    std::string(other.add.task);
			break;
		case CommandType::Remove:
			remove.index = other.remove.index;
			new (&remove.project) std::string(other.remove.project);
			new (&remove.tag)     std::string(other.remove.tag);
			break;
		case CommandType::Doo:
			doo.index = other.doo.index;
			doo.status = other.doo.status;
			new (&doo.project) std::string(other.doo.project);
			new (&doo.tag)     std::string(other.doo.tag);
			break;
		case CommandType::Set:
			set.index = other.set.index;
			set.status = other.set.status;
			break;
		default: break;
		}
		
		return *this;
	}
	
	~Command() {
		using string = std::string;
		
		switch (ct) {
		case CommandType::List:
			list.statuses.~set();
			list.project.~string();
			list.tags.~set();
			break;
		case CommandType::Add:
			add.project.~string();
			add.tag.~string();
			add.task.~string();
			break;
		case CommandType::Remove:
			remove.project.~string();
			remove.tag.~string();
			break;
		case CommandType::Doo:
			doo.project.~string();
			doo.tag.~string();
			break;
		default:
			break;
		}
	}
	
	CommandType type() const { return ct; }
	
	bool shouldUpdateHistory();
	void serialise(std::ostream &os);
	void deserialise(std::istream &is);
	void print();
	
	union {
		struct {
			ListMode mode;
			std::string project;
			std::set<std::string> tags;
			std::set<char> statuses;
		} list;
		
		struct {
			std::string project;
			std::string tag;
			std::string task;
			uint32_t index;
		} add;
		
		struct {
			std::string project;
			std::string tag;
			uint32_t index;
		} remove;
		
		struct {
			std::string project;
			std::string tag;
			uint32_t index;
			char status;
		} doo;

		struct {
			uint32_t index;
			char status;
		} set;
		
		struct {
			uint32_t placeholder;
		} undo;
		
		struct {
			uint32_t placeholder;
		} tidy;
		
		struct {
			uint32_t placeholder;
		} none;
	};
	
private:
	CommandType ct;
};

Command inverseCommand(const Todo &todo, const Command command);
void executeCommand(Todo &todo, Command &command);

#endif