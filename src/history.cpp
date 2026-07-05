#include "history.h"

#include "atomic_file.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {
const char *historyFormat = "TODO_HISTORY_V4";

void writeString(std::ostream &output, const std::string &value)
{
	output << value.size() << ',';
	output.write(value.data(), static_cast<std::streamsize>(value.size()));
	output << ',';
}

std::string readString(std::istream &input)
{
	std::size_t length = 0;
	char delimiter = '\0';
	if (!(input >> length >> delimiter) || delimiter != ',') {
		throw std::runtime_error("malformed string in command history");
	}

	std::string value(length, '\0');
	if (length > 0 &&
		!input.read(&value[0], static_cast<std::streamsize>(length))) {
		throw std::runtime_error("truncated string in command history");
	}
	if (!input.get(delimiter) || delimiter != ',') {
		throw std::runtime_error("malformed string in command history");
	}
	return value;
}

std::size_t readSize(std::istream &input)
{
	std::size_t value = 0;
	char delimiter = '\0';
	if (!(input >> value >> delimiter) || delimiter != ',') {
		throw std::runtime_error("malformed value in command history");
	}
	return value;
}
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
	if (!std::getline(file, format) || format != historyFormat) {
		return;
	}

	std::size_t entryCount = 0;
	if (!(file >> entryCount) || entryCount > maximumEntries) {
		throw std::runtime_error("invalid command history header");
	}

	entries.reserve(entryCount);
	for (std::size_t i = 0; i < entryCount; ++i) {
		const std::size_t lineCount = readSize(file);
		std::vector<std::string> lines;
		lines.reserve(lineCount);
		for (std::size_t j = 0; j < lineCount; ++j) {
			lines.push_back(readString(file));
		}
		entries.push_back(lines);
	}

	file >> std::ws;
	if (!file.eof()) {
		throw std::runtime_error("unexpected data at end of command history");
	}
}

const std::vector<std::string> &History::last() const
{
	if (entries.empty()) {
		throw std::runtime_error("no command history found");
	}
	return entries.back();
}

void History::push(const std::vector<std::string> &lines)
{
	if (entries.size() == maximumEntries) {
		entries.erase(entries.begin());
	}
	entries.push_back(lines);
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
	for (const std::vector<std::string> &lines : entries) {
		contents << lines.size() << ',';
		for (const std::string &line : lines) {
			writeString(contents, line);
		}
	}
	if (!contents) {
		throw std::runtime_error("failed to serialize command history");
	}
	atomicWriteFile(filename, contents.str());
}
