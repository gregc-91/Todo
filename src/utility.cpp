#include "utility.h"

#include <algorithm>
#include <cctype>
#include <vector>

namespace {
char lowerAscii(char value)
{
	return static_cast<char>(
		std::tolower(static_cast<unsigned char>(value)));
}
}

bool equalsIgnoreCase(const std::string &left, const std::string &right)
{
	if (left.size() != right.size()) {
		return false;
	}

	for (std::size_t i = 0; i < left.size(); ++i) {
		if (lowerAscii(left[i]) != lowerAscii(right[i])) {
			return false;
		}
	}
	return true;
}

std::size_t editDistance(const std::string &left, const std::string &right)
{
	std::vector<std::size_t> previous(right.size() + 1);
	std::vector<std::size_t> current(right.size() + 1);

	for (std::size_t j = 0; j <= right.size(); ++j) {
		previous[j] = j;
	}

	for (std::size_t i = 1; i <= left.size(); ++i) {
		current[0] = i;
		for (std::size_t j = 1; j <= right.size(); ++j) {
			const std::size_t substitution =
				previous[j - 1] +
				(lowerAscii(left[i - 1]) == lowerAscii(right[j - 1]) ? 0 : 1);
			current[j] = std::min({
				previous[j] + 1,
				current[j - 1] + 1,
				substitution
			});
		}
		previous.swap(current);
	}

	return previous[right.size()];
}
