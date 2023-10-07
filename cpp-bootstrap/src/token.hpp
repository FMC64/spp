#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <tuple>
#include <optional>
#include <stdexcept>

class File {
	std::filesystem::path m_filePath;
	std::string m_bytes;

	static std::string readFileBytes(const std::filesystem::path &filePath) {
		std::ifstream inputFile(filePath, std::ios::in | std::ios::binary);
		std::stringstream res;
		res << inputFile.rdbuf();
		return res.str();
	}

public:
	File(const std::filesystem::path &filePath) :
		m_filePath(filePath),
		m_bytes(readFileBytes(filePath)) {
	}

	bool isBeforeEnd(size_t offset) const {
		return offset < m_bytes.size();
	}

	// Must have `offset` not past end
	char read(size_t offset) const {
		return m_bytes[offset];
	}

	size_t getByteCount(void) const {
		return m_bytes.size();
	}
};

// Walk through the file, giving human readable current location at any time
class FileLocation {
	const File &m_pointedFile;

	// Byte offset, with zero the first byte of the file
	size_t m_offset;

	// Starts with one
	size_t m_line;
	// Starts with one, a tab is 8 columns
	size_t m_column;

public:
	FileLocation(const File &argPointedFile) :
		m_pointedFile(argPointedFile),
		m_offset(0),
		m_line(1),
		m_column(1) {
	}

	// Location getters
	size_t getOffset(void) const {
		return m_offset;
	}
	size_t getLine(void) const {
		return m_line;
	}
	size_t getColumn(void) const {
		return m_column;
	}

	// Modifiers
	// Must not be called if `!isBeforeEnd()`
	void moveForward(void) {
		auto nextCharacter = m_pointedFile.read(m_offset);
		m_offset++;

		if (nextCharacter == '\n') {
			m_line++;
			m_column = 1;
		} else if (nextCharacter == '\t') {
			m_column += 8;
		} else {
			m_column++;
		}
	}

	bool isBeforeEnd(void) const {
		return m_pointedFile.isBeforeEnd(m_offset);
	}

	// Must have `isBeforeEnd()` return `true`
	char getCurrentCharacter(void) const {
		return m_pointedFile.read(m_offset);
	}

	// Includes a unit for current character if before end
	size_t readableCharacterCount(void) const {
		return m_pointedFile.getByteCount() - m_offset;
	}

	// Must have `offset < readableCharacterCount()`
	char getNextCharacter(size_t offset) const {
		return m_pointedFile.read(m_offset + offset);
	}

	// Must have `characterCount <= readableCharacterCount()`
	void moveForwardMultiple(size_t characterCount) {
		for (size_t i = 0; i < characterCount; i++)
			moveForward();
	}
};

enum class TokenClass {
	Layout,
	Operator,
	Digits,
	Identifier,
	StringLiteral
};

// Token without a location in a file
class TokenStub {
	TokenClass m_class;
	std::string m_underlyingStr;

public:
	TokenStub(TokenClass tokenClass, const std::string &str) :
		m_class(tokenClass),
		m_underlyingStr(str) {
	}

	TokenClass getClass(void) const {
		return m_class;
	}

	const std::string& getString(void) const {
		return m_underlyingStr;
	}
};

class Token {
	FileLocation m_fileLocation;
	TokenClass m_class;
	std::string m_underlyingStr;

	static inline std::string linefeedString = "\n";
	static inline std::string escapedLinefeedString = "[LINEFEED]";

public:
	Token(const FileLocation &fileLocation, TokenClass tokenClass, const std::string &str) :
		m_fileLocation(fileLocation),
		m_class(tokenClass),
		m_underlyingStr(str) {
	}
	Token(const FileLocation &fileLocation, const TokenStub &stub) :
		Token(fileLocation, stub.getClass(), stub.getString()) {
	}

	const FileLocation& getFileLocation(void) const {
		return m_fileLocation;
	}

	TokenClass getClass(void) const {
		return m_class;
	}

	size_t getLength(void) const {
		return m_underlyingStr.size();
	}

	const std::string& getEscapedString(void) const {
		if (m_underlyingStr == linefeedString)
			return escapedLinefeedString;
		else
			return m_underlyingStr;
	}
};

namespace Tokens {
	// Layout
	static inline auto linefeed = TokenStub(TokenClass::Layout, "\n");

	// Scope operators
	static inline auto dot = TokenStub(TokenClass::Operator, ".");
	static inline auto leftParenthesis = TokenStub(TokenClass::Operator, "(");
	static inline auto rightParenthesis = TokenStub(TokenClass::Operator, ")");
	static inline auto leftArraySubscript = TokenStub(TokenClass::Operator, "[");
	static inline auto rightArraySubscript = TokenStub(TokenClass::Operator, "]");
	static inline auto leftBracket = TokenStub(TokenClass::Operator, "{");
	static inline auto rightBracket = TokenStub(TokenClass::Operator, "}");
	static inline auto comma = TokenStub(TokenClass::Operator, ",");
	static inline auto colon = TokenStub(TokenClass::Operator, ":");
	static inline auto semicolon = TokenStub(TokenClass::Operator, ";");
	static inline auto variableArgumentCountType = TokenStub(TokenClass::Operator, "...");
	static inline auto assign = TokenStub(TokenClass::Operator, "<-");

	// Arithmetic
	static inline auto booleanNot = TokenStub(TokenClass::Operator, "!");
	static inline auto binaryNot = TokenStub(TokenClass::Operator, "~");
	static inline auto plus = TokenStub(TokenClass::Operator, "+");
	static inline auto minus = TokenStub(TokenClass::Operator, "-");
	static inline auto increment = TokenStub(TokenClass::Operator, "++");
	static inline auto decrement = TokenStub(TokenClass::Operator, "--");

	static inline auto multiplication = TokenStub(TokenClass::Operator, "*");
	static inline auto division = TokenStub(TokenClass::Operator, "/");
	static inline auto modulo = TokenStub(TokenClass::Operator, "%");

	// Binary
	static inline auto shiftedToLeftBy = TokenStub(TokenClass::Operator, "<<");
	static inline auto shiftedToRightBy = TokenStub(TokenClass::Operator, ">>");
	static inline auto binaryOr = TokenStub(TokenClass::Operator, "|");
	static inline auto binaryAnd = TokenStub(TokenClass::Operator, "&");
	static inline auto binaryXor = TokenStub(TokenClass::Operator, "^");

	// Comparison
	static inline auto equalTo = TokenStub(TokenClass::Operator, "=");
	static inline auto differentFrom = TokenStub(TokenClass::Operator, "=/=");
	static inline auto greaterThan = TokenStub(TokenClass::Operator, ">");
	static inline auto lesserThan = TokenStub(TokenClass::Operator, "<");
	static inline auto greaterThanOrEqualTo = TokenStub(TokenClass::Operator, ">_");
	static inline auto lesserThanOrEqualTo = TokenStub(TokenClass::Operator, "_<");

	static inline std::vector<TokenStub> allOperators = {
		dot,
		leftParenthesis,
		rightParenthesis,
		leftArraySubscript,
		rightArraySubscript,
		leftBracket,
		rightBracket,
		comma,
		colon,
		semicolon,
		variableArgumentCountType,
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
	static void skipWhitespace(FileLocation &currentLocation) {
		while (currentLocation.isBeforeEnd() && isWhitespace(currentLocation.getCurrentCharacter())) {
			currentLocation.moveForward();
		}
	}

	// Will have result index after the next linefeed or at EOF
	static void skipLine(FileLocation &currentLocation) {
		while (currentLocation.isBeforeEnd()) {
			if (currentLocation.getCurrentCharacter() == '\n') {
				currentLocation.moveForward();
				return;
			}

			currentLocation.moveForward();
		}
	}

	static bool doesFileContainStringAt(FileLocation &currentLocation, const std::string &toFind) {
		if (currentLocation.readableCharacterCount() < toFind.size())
			return false;

		for (size_t i = 0; i < toFind.size(); i++)
			if (currentLocation.getNextCharacter(i) != toFind[i])
				return false;
		return true;
	}

	static inline std::string singleLineComment = "//";
	static inline std::string multiLineCommentBegin = "/*";
	static inline std::string multiLineCommentEnd = "*/";

	static void skipComment(FileLocation &currentLocation) {
		if (doesFileContainStringAt(currentLocation, singleLineComment)) {
			skipLine(currentLocation);
			return;
		} else if (doesFileContainStringAt(currentLocation, multiLineCommentBegin)) {
			while (currentLocation.isBeforeEnd()) {
				if (doesFileContainStringAt(currentLocation, multiLineCommentEnd)) {
					currentLocation.moveForwardMultiple(multiLineCommentEnd.size());
					return;
				} else {
					currentLocation.moveForward();
				}
			}
		}
	}

	static void getNextTokenOffsetFrom(FileLocation &currentLocation) {
		while (currentLocation.isBeforeEnd()) {
			skipWhitespace(currentLocation);
			auto preCommentOffset = currentLocation.getOffset();
			skipComment(currentLocation);
			// If no comment has been detected, we got our next token right there
			if (currentLocation.getOffset() == preCommentOffset)
				break;
		}
	}

	static Token pollString(FileLocation &currentLocation) {
		auto delimiter = currentLocation.getCurrentCharacter();
		// Make a copy of the location of the beginning of the string
		auto beginLocation = currentLocation;
		// Skip opening delimiter
		currentLocation.moveForward();

		std::string string;
		while (true) {
			if (!currentLocation.isBeforeEnd()) {
				std::stringstream ss;
				ss << "from offset " << beginLocation.getOffset() << ": unterminated string";
				throw std::runtime_error(ss.str());
			}
			if (currentLocation.getCurrentCharacter() == delimiter) {
				currentLocation.moveForward();
				return Token(beginLocation, TokenClass::StringLiteral, string);
			} else {
				string.push_back(currentLocation.getCurrentCharacter());
			}
			currentLocation.moveForward();
		}
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

	static Token pollCharSequence(FileLocation &currentLocation) {
		auto beginLocation = currentLocation;
		auto firstChar = currentLocation.getCurrentCharacter();
		auto isDigit = isCharDigit(firstChar);
		std::string sequence;

		while (currentLocation.isBeforeEnd() && (isCharAlphanum(currentLocation.getCurrentCharacter()) || currentLocation.getCurrentCharacter() == '_')) {
			sequence.push_back(currentLocation.getCurrentCharacter());
			currentLocation.moveForward();
		}
		return Token(beginLocation, isDigit ? TokenClass::Digits : TokenClass::Identifier, sequence);
	}

	static Token getTokenAt(FileLocation &currentLocation) {
		auto firstChar = currentLocation.getCurrentCharacter();
		// Linefeed
		{
			if (firstChar == '\n') {
				auto res = Token(currentLocation, Tokens::linefeed);
				currentLocation.moveForward();
				return res;
			}
		}

		// String literal
		{
			if (firstChar == '\'' || firstChar == '"')
				return pollString(currentLocation);
		}

		// Operators
		{
			struct Best {
				size_t length;
				TokenStub token;
			};
			std::optional<Best> bestOperator;

			for (auto &op : Tokens::allOperators) {
				if (!doesFileContainStringAt(currentLocation, op.getString()))
					continue;

				auto currentOpLength = op.getString().size();
				if (!bestOperator.has_value() || currentOpLength > bestOperator->length) {
					bestOperator = Best{
						.length = currentOpLength,
						.token = op
					};
				}
			}
			if (bestOperator.has_value()) {
				auto res = Token(currentLocation, bestOperator->token);
				currentLocation.moveForwardMultiple(bestOperator->length);
				return res;
			}
		}

		// Fallback to char sequence
		{
			return pollCharSequence(currentLocation);
		}
	}

public:
	static std::vector<Token> readTokens(const File &sourceFile) {
		auto currentLocation = FileLocation(sourceFile);
		std::vector<Token> res;

		while (currentLocation.isBeforeEnd()) {
			getNextTokenOffsetFrom(currentLocation);
			if (currentLocation.isBeforeEnd()) {
				auto token = getTokenAt(currentLocation);

				if (token.getLength() == 0) {
					std::stringstream ss;
					ss << "offset " << currentLocation.getOffset() << ": illegal character";
					throw std::runtime_error(ss.str());
				}
				res.emplace_back(std::move(token));
			}
		}
		return res;
	}
};