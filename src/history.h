#ifndef TODO_HISTORY_H
#define TODO_HISTORY_H

#include "command.h"

#include <cstddef>
#include <string>
#include <vector>

struct HistoryEntry {
	Command command;
	Command inverse;
};

class History {
public:
	explicit History(const std::string &filename);

	bool empty() const;
	const HistoryEntry &last() const;
	void push(const Command &command, const Command &inverse);
	void pop();
	void commit() const;

private:
	static constexpr std::size_t maximumEntries = 100;

	std::string filename;
	std::vector<HistoryEntry> entries;
};

#endif
