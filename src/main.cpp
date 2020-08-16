
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>

enum CommandType {
	List,
	Add,
	Remove,
	Doo,
	Tidy,
	None,
	CommandTypeSize
};

const char* CommandStrings[CommandType::CommandTypeSize] = {
	"List",
	"Add",
	"Remove",
	"Do",
	"Tidy",
	"None"
};

struct Command {
	Command() : ct(CommandType::None) {
	}
	
	Command(CommandType commandType) {
		ct = commandType;
		switch (ct) {
		case CommandType::List: 
			new (&list.project) std::string();
			new (&list.tag)     std::string();
			break;
		case CommandType::Add:
			new (&add.project) std::string();
			new (&add.tag)     std::string();
			new (&add.task)    std::string();
			break;
		case CommandType::Remove:
			new (&remove.project) std::string();
			new (&remove.tag)     std::string();
			break;
		case CommandType::Doo:
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
			new (&list.project) std::string(other.list.project);
			new (&list.tag)     std::string(other.list.tag);
			break;
		case CommandType::Add:
			new (&add.project) std::string(other.add.project);
			new (&add.tag)     std::string(other.add.tag);
			new (&add.task)    std::string(other.add.task);
			break;
		case CommandType::Remove:
			new (&remove.project) std::string(other.remove.project);
			new (&remove.tag)     std::string(other.remove.tag);
			break;
		case CommandType::Doo:
			new (&doo.project) std::string(other.doo.project);
			new (&doo.tag)     std::string(other.doo.tag);
			break;
		default: break;
		}		
	}
	
	Command operator=(const Command& other) {
		return Command(other);
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
			enum {
				Tasks,
				Projects,
				Tags
			} mode;
			std::string project;
			std::string tag;
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

Command parseCommandType(char* str) {
	
	printf("%s %s\n", str, CommandStrings[CommandType::Add]);
	
	if (!strcmp(str, CommandStrings[CommandType::List]))   return Command(CommandType::List);
	if (!strcmp(str, CommandStrings[CommandType::Add]))    return Command(CommandType::Add);
	if (!strcmp(str, CommandStrings[CommandType::Remove])) return Command(CommandType::Remove);
	if (!strcmp(str, CommandStrings[CommandType::Doo]))    return Command(CommandType::Doo);
	if (!strcmp(str, CommandStrings[CommandType::Tidy]))   return Command(CommandType::Tidy);

	throw std::invalid_argument("Invalid command type!");
}

Command parseCommand(int argc, char** argv) {
	if (argc < 2)  throw std::invalid_argument("Not enough arguments!");
	
	Command command = parseCommandType(argv[1]);

	return command;
}

void executeListCommand(const Command command) {
	printf("Executing list command.\n");
};

void executeAddCommand(const Command command) {
	printf("Executing add command.\n");
};

void executeRemoveCommand(const Command command) {
	printf("Executing remove command.\n");
};

void executeDooCommand(const Command command) {
	printf("Executing do command.\n");
};

void executeTidyCommand(const Command command) {
	printf("Executing tidy command.\n");
};

void executeCommand(const Command command) {
	
	switch (command.type()) {
	case CommandType::List:
		{
			executeListCommand(command);
		} break;
	case CommandType::Add:
		{
			executeAddCommand(command);
		} break;
	case CommandType::Remove:
		{
			executeRemoveCommand(command);
		} break;
	case CommandType::Doo:
		{
			executeDooCommand(command);
		} break;
	case CommandType::Tidy:
		{
			executeTidyCommand(command);
		} break;
	}
}

void usage() {
	printf("usage: todo <command> [options]\n");
}

int main(int argc, char** argv) {
	
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
