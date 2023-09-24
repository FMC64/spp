#include <cstdio>
#include <vector>
#include <string>
#include <stdexcept>
#include <map>
#include <set>
#include "compiler.hpp"
#include "runner.hpp"

int main(int argc, char **argv) {
	enum class Flag {
		Inspect
	};
	static std::map<std::string, Flag> stringToFlag {
		{"-i", Flag::Inspect},
		{"--inspect", Flag::Inspect}
	};

	try {
		std::vector<std::string> args;
		for (int i = 1; i < argc; i++)
			args.emplace_back(argv[i]);

		std::set<Flag> flags;
		size_t currentArg = 0;
		for (; currentArg < args.size(); currentArg++) {
			auto &arg = args[currentArg];
			if (arg.size() < 1)
				break;
			if (arg[0] != '-')
				break;
			flags.emplace(stringToFlag.at(arg));
		}
		if (!(currentArg < args.size()))
			throw std::runtime_error("Expected at least a single argument after flags");
		auto &entrypointPath = args[currentArg++];

		std::vector<std::string> runnerArgs;
		while (currentArg < args.size())
			runnerArgs.emplace_back(args[currentArg++]);

		auto compiler = Compiler();
		auto program = compiler.build(entrypointPath);

		if (flags.contains(Flag::Inspect))
			program.inspect();
		else {
			auto runner = Runner();
			runner.run(program, runnerArgs);
		}

		return 0;
	} catch (const std::exception &error) {
		std::fprintf(stderr, "FATAL ERROR: %s\n", error.what());
		return 1;
	}
}