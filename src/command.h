#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <assert.h>
#include <string>

namespace Colour {
	__attribute__ ((unused)) static const char* Black        = "\u001b[30m";
	__attribute__ ((unused)) static const char* Red          = "\u001b[31m";
	__attribute__ ((unused)) static const char* Green        = "\u001b[32m";
	__attribute__ ((unused)) static const char* Yellow       = "\u001b[33m";
	__attribute__ ((unused)) static const char* Blue         = "\u001b[34m";
	__attribute__ ((unused)) static const char* Pink         = "\u001b[35m";
	__attribute__ ((unused)) static const char* Cyan         = "\u001b[36m";
	__attribute__ ((unused)) static const char* White        = "\u001b[37m";
	__attribute__ ((unused)) static const char* BrightBlack  = "\u001b[30;1m";
	__attribute__ ((unused)) static const char* BrightRed    = "\u001b[31;1m";
	__attribute__ ((unused)) static const char* BrightGreen  = "\u001b[32;1m";
	__attribute__ ((unused)) static const char* BrightYellow = "\u001b[33;1m";
	__attribute__ ((unused)) static const char* BrightBlue   = "\u001b[34;1m";
	__attribute__ ((unused)) static const char* BrightPink   = "\u001b[35;1m";
	__attribute__ ((unused)) static const char* BrightCyan   = "\u001b[36;1m";
	__attribute__ ((unused)) static const char* BrightWhite  = "\u001b[37;1m";
	__attribute__ ((unused)) static const char* Orange       = "\u001b[38;5;208m";
	
	__attribute__ ((unused)) static const char* Reset  = "\u001b[0m";
}

enum CommandType {
	List,
	Add,
	Remove,
	Doo,
	Tidy,
	None,
	CommandTypeSize
};

extern const char* CommandStrings[CommandType::CommandTypeSize];

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

enum ListMode {
	Tasks,
	Projects,
	Tags
};

struct Command {
	Command() : ct(CommandType::None) {
	}
	
	Command(CommandType commandType) {
		ct = commandType;
		switch (ct) {
		case CommandType::List: 
			list.mode = ListMode::Tasks;
			list.status = '\0';
			new (&list.project) std::string();
			new (&list.tag)     std::string();
			break;
		case CommandType::Add:
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
			new (&doo.project) std::string();
			new (&doo.tag)     std::string();
			break;
		default: break;
		}		
	}
	
	Command(const Command& other) {
		ct = other.type();
		switch (ct) {
		case CommandType::List: 
			list.mode = other.list.mode;
			list.status = other.list.status;
			new (&list.project) std::string(other.list.project);
			new (&list.tag)     std::string(other.list.tag);
			break;
		case CommandType::Add:
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
			new (&doo.project) std::string(other.doo.project);
			new (&doo.tag)     std::string(other.doo.tag);
			break;
		default: break;
		}		
	}
	
	Command& operator=(const Command& other) {
		assert(ct == CommandType::None);
		ct = other.type();
		switch (ct) {
		case CommandType::List: 
			list.mode = other.list.mode;
			list.status = other.list.status;
			new (&list.project) std::string(other.list.project);
			new (&list.tag)     std::string(other.list.tag);
			break;
		case CommandType::Add:
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
			new (&doo.project) std::string(other.doo.project);
			new (&doo.tag)     std::string(other.doo.tag);
			break;
		default: break;
		}
		
		return *this;
	}
	
	~Command() {
		using string = std::string;
		
		switch (ct) {
		case CommandType::List:
			list.project.~string();
			list.tag.~string();
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
	
	union {
		struct {
			ListMode mode;
			std::string project;
			std::string tag;
			char status;
		} list;
		
		struct {
			std::string project;
			std::string tag;
			std::string task;
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
		} doo;
		
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

void executeCommand(const Command command);

#endif