#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <tuple>
#include <optional>
#include <stdexcept>

enum class TokenClass {
	Layout,
	Operator,
	Digits,
	Identifier,
	StringLiteral
};

class Token {
	TokenClass _class;
	std::string _underlyingStr;

	static inline std::string linefeedString = "\n";
	static inline std::string escapedLinefeedString = "\\n";

public:
	Token(TokenClass tokenClass, const std::string &str) :
		_class(tokenClass),
		_underlyingStr(str) {
	}

	size_t getLength(void) const {
		return _underlyingStr.size();
	}

	const std::string& getEscapedString(void) const {
		if (_underlyingStr == linefeedString)
			return escapedLinefeedString;
		else
			return _underlyingStr;
	}
};

namespace Tokens {
	// Layout
	static inline auto linefeed = Token(TokenClass::Layout, "\n");

	// Scope operators
	static inline auto dot = Token(TokenClass::Operator, ".");
	static inline auto leftParenthesis = Token(TokenClass::Operator, "(");
	static inline auto rightParenthesis = Token(TokenClass::Operator, ")");
	static inline auto leftArraySubscript = Token(TokenClass::Operator, "[");
	static inline auto rightArraySubscript = Token(TokenClass::Operator, "]");
	static inline auto leftBracket = Token(TokenClass::Operator, "{");
	static inline auto rightBracket = Token(TokenClass::Operator, "}");
	static inline auto colon = Token(TokenClass::Operator, ":");
	static inline auto assign = Token(TokenClass::Operator, "<-");

	// Arithmetic
	static inline auto booleanNot = Token(TokenClass::Operator, "!");
	static inline auto binaryNot = Token(TokenClass::Operator, "~");
	static inline auto plus = Token(TokenClass::Operator, "+");
	static inline auto minus = Token(TokenClass::Operator, "-");
	static inline auto increment = Token(TokenClass::Operator, "++");
	static inline auto decrement = Token(TokenClass::Operator, "--");

	static inline auto multiplication = Token(TokenClass::Operator, "*");
	static inline auto division = Token(TokenClass::Operator, "/");
	static inline auto modulo = Token(TokenClass::Operator, "%");

	// Binary
	static inline auto shiftedToLeftBy = Token(TokenClass::Operator, "<<");
	static inline auto shiftedToRightBy = Token(TokenClass::Operator, ">>");
	static inline auto binaryOr = Token(TokenClass::Operator, "|");
	static inline auto binaryAnd = Token(TokenClass::Operator, "&");
	static inline auto binaryXor = Token(TokenClass::Operator, "^");

	// Comparison
	static inline auto equalTo = Token(TokenClass::Operator, "=");
	static inline auto differentFrom = Token(TokenClass::Operator, "=/=");
	static inline auto greaterThan = Token(TokenClass::Operator, ">");
	static inline auto lesserThan = Token(TokenClass::Operator, "<");
	static inline auto greaterThanOrEqualTo = Token(TokenClass::Operator, ">_");
	static inline auto lesserThanOrEqualTo = Token(TokenClass::Operator, "_<");

	static inline std::vector<Token> allOperators = {
		dot,
		leftParenthesis,
		rightParenthesis,
		leftArraySubscript,
		rightArraySubscript,
		leftBracket,
		rightBracket,
		colon,
		assign,

		booleanNot,
		binaryNot,
		plus,
		minus,
		increment,
		decrement,

		multiplication,
		division,
		modulo,

		shiftedToLeftBy,
		shiftedToRightBy,
		binaryOr,
		binaryAnd,
		binaryXor,

		equalTo,
		differentFrom,
		greaterThan,
		lesserThan,
		greaterThanOrEqualTo,
		lesserThanOrEqualTo
	};
}

class TokenParser {
	// WARNNING: will not skip over linefeeds!
	static inline bool isWhitespace(char candidate) {
		if (candidate == '\n')
			return false;
		else
			return candidate <= ' ' || candidate >= 0x7F;
	}

	// Will have result index after the last whitespace from `offset` or at EOF
	// WARNNING: will not skip over linefeeds!
	static size_t skipWhitespace(const std::string &sourceFile, size_t offset) {
		while (offset < sourceFile.size() && isWhitespace(sourceFile[offset])) {
			offset++;
		}
		return offset;
	}

	// Will have result index after the next linefeed or at EOF
	static size_t skipLine(const std::string &sourceFile, size_t offset) {
		while (offset < sourceFile.size()) {
			if (sourceFile[offset] == '\n') {
				return offset + 1;
			}

			offset++;
		}
		return offset;
	}

	static bool doesFileContainStringAt(const std::string &sourceFile, size_t offset, const std::string &toFind) {
		if (offset + toFind.size() < sourceFile.size()) {
			for (size_t i = 0; i < toFind.size(); i++)
				if (sourceFile[offset + i] != toFind[i])
					return false;
			return true;
		} else
			return false;
	}

	static inline std::string singleLineComment = "//";
	static inline std::string multiLineCommentBegin = "/*";
	static inline std::string multiLineCommentEnd = "*/";

	static size_t skipComment(const std::string &sourceFile, size_t offset) {
		if (doesFileContainStringAt(sourceFile, offset, singleLineComment)) {
			return skipLine(sourceFile, offset);
		} else if (doesFileContainStringAt(sourceFile, offset, multiLineCommentBegin)) {
			while (offset < sourceFile.size() && doesFileContainStringAt(sourceFile, offset, multiLineCommentEnd)) {
				offset++;
			}
			return offset;
		} else
			return offset;
	}

	static size_t getNextTokenOffsetFrom(const std::string &sourceFile, size_t offset) {
		while (offset < sourceFile.size()) {
			offset = skipWhitespace(sourceFile, offset);
			auto newOffset = skipComment(sourceFile, offset);
			if (newOffset == offset)
				break;
			offset = newOffset;
		}
		return offset;
	}

	static char toLower(char toProcess) {
		if (toProcess >= 'A' && toProcess <= 'Z')
			return toProcess + 32;
		else
			return toProcess;
	}

	static bool isCharDigit(char candidate) {
		return candidate >= '0' && candidate <= '9';
	}

	static bool isCharAlphanum(char candidate) {
		candidate = toLower(candidate);
		return (candidate >= 'a' && candidate <= 'z') || isCharDigit(candidate);
	}

	static std::pair<size_t, Token> pollCharSequence(const std::string &sourceFile, size_t offset) {
		auto firstChar = sourceFile[offset];
		auto isDigit = isCharDigit(firstChar);
		std::string sequence;

		while (offset < sourceFile.size() && (isCharAlphanum(sourceFile[offset]) || sourceFile[offset] == '_')) {
			sequence.push_back(sourceFile[offset]);
			offset++;
		}
		return std::make_pair(offset, Token(isDigit ? TokenClass::Digits : TokenClass::Identifier, sequence));
	}

	static std::pair<size_t, Token> getTokenAt(const std::string &sourceFile, size_t offset) {
		// Linefeed
		{
			auto firstChar = sourceFile[offset];
			if (firstChar == '\n')
				return std::make_pair(offset + 1, Tokens::linefeed);
		}

		// Operators
		{
			struct Best {
				size_t length;
				Token token;
			};
			std::optional<Best> bestOperator;

			for (auto &op : Tokens::allOperators) {
				if (!doesFileContainStringAt(sourceFile, offset, op.getEscapedString()))
					continue;

				auto currentOpLength = op.getLength();
				if (!bestOperator.has_value() || currentOpLength > bestOperator->length) {
					bestOperator = Best{
						.length = currentOpLength,
						.token = op
					};
				}
			}
			if (bestOperator.has_value())
				return std::make_pair(offset + bestOperator->length, bestOperator->token);
		}

		// Fallback to char sequence
		{
			return pollCharSequence(sourceFile, offset);
		}
	}

	static std::string readFile(const std::filesystem::path &filePath) {
		std::ifstream inputFile(filePath, std::ios::in);
		std::stringstream res;
		res << inputFile.rdbuf();
		return res.str();
	}

public:
	static std::vector<Token> readTokens(const std::filesystem::path &sourceFilePath) {
		auto sourceFile = readFile(sourceFilePath);

		std::vector<Token> res;
		size_t offset = 0;

		while (offset < sourceFile.size()) {
			offset = getNextTokenOffsetFrom(sourceFile, offset);
			if (offset < sourceFile.size()) {
				auto [nextTokenOffset, token] = getTokenAt(sourceFile, offset);

				if (token.getLength() == 0) {
					std::stringstream ss;
					ss << sourceFilePath << ", offset " << offset << ": illegal character";
					throw std::runtime_error(ss.str());
				}

				offset = nextTokenOffset;
				res.emplace_back(std::move(token));
			}
		}
		return res;
	}
};