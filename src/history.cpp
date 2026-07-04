#include "history.h"

#include "atomic_file.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {
const char *historyFormat = "TODO_HISTORY_V3";
}

History::History(const std::string &filename) :
	filename(filename)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file) {
		if (std::filesystem::exists(filename)) {
			throw std::runtime_error(
				"unable to open command history '" + filename + "'");
		}
		return;
	}

	std::string format;
	if (!std::getline(file, format)) {
		return;
	}

	// Earlier releases stored one unframed command pair. Those inverse
	// commands cannot reliably restore line positions, so start a safe stack.
	if (format != historyFormat) {
		return;
	}

	std::size_t entryCount = 0;
	if (!(file >> entryCount) || entryCount > maximumEntries) {
		throw std::runtime_error("invalid command history header");
	}

	entries.reserve(entryCount);
	for (std::size_t i = 0; i < entryCount; ++i) {
		HistoryEntry entry;
		entry.command.deserialise(file);
		entry.inverse.deserialise(file);
		entries.push_back(entry);
	}

	file >> std::ws;
	if (!file.eof()) {
		throw std::runtime_error("unexpected data at end of command history");
	}
}

bool History::empty() const
{
	return entries.empty();
}

const HistoryEntry &History::last() const
{
	if (entries.empty()) {
		throw std::runtime_error("no command history found");
	}
	return entries.back();
}

void History::push(const Command &command, const Command &inverse)
{
	if (entries.size() == maximumEntries) {
		entries.erase(entries.begin());
	}
	entries.push_back({command, inverse});
}

void History::pop()
{
	if (entries.empty()) {
		throw std::runtime_error("no command history found");
	}
	entries.pop_back();
}

void History::commit() const
{
	std::ostringstream contents;
	contents << historyFormat << '\n' << entries.size() << '\n';
	for (const HistoryEntry &entry : entries) {
		entry.command.serialise(contents);
		entry.inverse.serialise(contents);
	}
	if (!contents) {
		throw std::runtime_error("failed to serialize command history");
	}
	atomicWriteFile(filename, contents.str());
}
