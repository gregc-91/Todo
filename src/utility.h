#ifndef TODO_UTILITY_H
#define TODO_UTILITY_H

#include <cstddef>
#include <string>

bool equalsIgnoreCase(const std::string &left, const std::string &right);
std::size_t editDistance(const std::string &left, const std::string &right);

#endif
