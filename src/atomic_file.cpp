#include "atomic_file.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {
void removeTemporaryFile(const std::filesystem::path &temporary)
{
	std::error_code ignored;
	std::filesystem::remove(temporary, ignored);
}

void replaceFile(const std::filesystem::path &temporary,
				 const std::filesystem::path &target)
{
#ifdef _WIN32
	if (!MoveFileExW(temporary.c_str(), target.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
		const unsigned long error = GetLastError();
		throw std::runtime_error(
			"unable to replace '" + target.string() +
			"' (Windows error " + std::to_string(error) + ")");
	}
#else
	std::error_code error;
	std::filesystem::rename(temporary, target, error);
	if (error) {
		throw std::runtime_error(
			"unable to replace '" + target.string() +
			"': " + error.message());
	}
#endif
}
}

void atomicWriteFile(const std::string &filename, const std::string &contents)
{
	const std::filesystem::path target(filename);
	std::filesystem::path temporary(target);
	temporary += ".tmp";

	try {
		{
			std::ofstream output(
				temporary, std::ios::binary | std::ios::trunc);
			if (!output) {
				throw std::runtime_error(
					"unable to open temporary file '" +
					temporary.string() + "'");
			}

			output.write(contents.data(),
				static_cast<std::streamsize>(contents.size()));
			output.flush();
			if (!output) {
				throw std::runtime_error(
					"failed while writing temporary file '" +
					temporary.string() + "'");
			}

			output.close();
			if (!output) {
				throw std::runtime_error(
					"failed to close temporary file '" +
					temporary.string() + "'");
			}
		}

		if (std::filesystem::exists(target)) {
			std::error_code error;
			const auto permissions =
				std::filesystem::status(target, error).permissions();
			if (error) {
				throw std::runtime_error(
					"unable to read permissions for '" +
					target.string() + "': " + error.message());
			}
			std::filesystem::permissions(
				temporary, permissions,
				std::filesystem::perm_options::replace, error);
			if (error) {
				throw std::runtime_error(
					"unable to preserve permissions for '" +
					target.string() + "': " + error.message());
			}
		}

		replaceFile(temporary, target);
	} catch (...) {
		removeTemporaryFile(temporary);
		throw;
	}
}
