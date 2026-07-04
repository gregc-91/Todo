#ifndef TODO_ATOMIC_FILE_H
#define TODO_ATOMIC_FILE_H

#include <string>

void atomicWriteFile(const std::string &filename, const std::string &contents);

#endif
