
#include <iostream>
#include <string>

enum CommandType {
	List,
	Add,
	Remove,
	Doo,
	Tidy,
	None
};

struct Command {
	Command() : ct(CommandType::None) {
	}
	
	Command(const Command& other) {
		ct = other.ct;
		switch (ct) {
		case CommandType::List:   list   = other.list;   break;
		case CommandType::Add:    add    = other.add;    break;
		case CommandType::Remove: remove = other.remove; break;
		case CommandType::Doo:    doo    = other.doo;    break;
		case CommandType::Tidy:   tidy   = other.tidy;   break;
		case CommandType::None:   none   = other.none;   break;
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
			list.project.~string();
			list.tag.~string();
			break;
		case CommandType::Doo:
			list.project.~string();
			list.tag.~string();
			break;
		default:
			break;
		}
	}
	
	CommandType ct;
	
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
};

Command parseCommand(int argc, char** argv) {
	Command command;
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
	
	switch (command.ct) {
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
	
}

int main(int argc, char** argv) {
	
	Command command;

	try {
		command = parseCommand(argc, argv);
	} catch(std::invalid_argument &e) {
		std::cout << e.what() << std::endl;
		exit(-1);
	}
	
	try {
		executeCommand(command);
	} catch(...) {
		//Todo: handle execution execeptions
		std::cout << "Error" << std::endl;
		exit(-1);
	}
	
	std::cout << "Success!" << std::endl;
}
