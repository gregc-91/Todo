#ifndef __HISTORY_H__
#define __HISTORY_H__

#include "todo.h"
#include "command.h"

class History {
	
	Command     last_command;
	Command     inverse_command;   
	std::string filename;
	
	History(std::string filename);
	
	void update(const Todo &todo, const Command &command);
	void commit();
};

#endif