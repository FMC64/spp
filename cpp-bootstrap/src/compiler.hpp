#pragma once

#include <filesystem>
#include "program.hpp"

class Compiler
{
public:
	Compiler(void) {
	}

	Program build(const std::filesystem::path &entryPointPath) {
		return Program();
	}
};