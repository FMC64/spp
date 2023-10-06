#pragma once

#include <filesystem>
#include "token.hpp"
#include "program.hpp"

#include <cstdio>

class Compiler
{
public:
	Compiler(void) {
	}

	Program build(const std::filesystem::path &entryPointPath) {
		auto tokens = TokenParser::readTokens(entryPointPath);
		for (auto &token : tokens) {
			std::printf("'%s'\n", token.getEscapedString().c_str());
		}

		return Program();
	}
};