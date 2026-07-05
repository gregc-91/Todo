#ifndef TODO_HISTORY_H
#define TODO_HISTORY_H

#include <cstddef>
#include <string>
#include <vector>

class History {
public:
	explicit History(const std::string &filename);

	const std::vector<std::string> &last() const;
	void push(const std::vector<std::string> &lines);
	void pop();
	void commit() const;

private:
	static constexpr std::size_t maximumEntries = 100;

	std::string filename;
	std::vector<std::vector<std::string>> entries;
};

#endif
