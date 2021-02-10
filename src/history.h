#ifndef __HISTORY_H__
#define __HISTORY_H__

#include "Todo.h"
#include "Command.h"

class History {
	
	Command     last_command;
	Command     inverse_command;   
	std::string filename;
	
	History(std::string filename);
	
	void update(const Todo &todo, const Command &command);
	void commit();
};

#endif