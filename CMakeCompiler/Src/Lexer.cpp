#include "CMakeCompiler/Lexer.h"

#include <utility>

namespace CMakeCompiler {
	SourceRef::SourceRef(std::size_t index, std::size_t line, std::size_t column)
	    : m_Index(index), m_Line(line), m_Column(column) { }

	LexError::LexError(const std::string& message, const SourceRef& at)
	    : m_Message(message), m_At(at) { }

	LexError::LexError(std::string&& message, SourceRef&& at)
	    : m_Message(std::move(message)), m_At(std::move(at)) { }

	bool lexFile(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// fileElement*
		SourceRef currentRef = begin;
		while (true) {
			LexNode elementNode;
			if (!lexFileElement(str, currentRef, end, elementNode, errors))
				break;
			node.m_Children.push_back(std::move(elementNode));
			currentRef = end;
		}
		node.m_Type  = LexNodeType::File;
		node.m_Begin = begin;
		node.m_End   = end;
		return true;
	}

	bool lexFileElement(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// commandInvocation lineEnding
		auto variant1 = [&]() -> bool {
			std::string_view tempStr = str;
			SourceRef tempEnd        = begin;
			LexNode commandInvocation;
			LexNode lineEnding;
			std::vector<LexError> tempErrors;

			if (!lexCommandInvocation(tempStr, begin, tempEnd, commandInvocation, tempErrors))
				return false;

			for (auto& error : tempErrors)
				errors.push_back(std::move(error));

			tempStr = tempStr.substr(tempEnd.m_Index - begin.m_Index);
			if (!lexLineEnding(tempStr, tempEnd, tempEnd, lineEnding, errors))
				return false;

			end = tempEnd;
			node.m_Children.push_back(std::move(commandInvocation));
			node.m_Children.push_back(std::move(lineEnding));
			return true;
		};

		// (bracketComment | space)* lineEnding
		auto variant2 = [&]() -> bool {
			std::string_view tempStr = str;
			SourceRef tempBegin      = begin;
			SourceRef tempEnd        = begin;
			std::vector<LexNode> tempNodes;
			LexNode lineEnding;
			std::vector<LexError> tempErrors;

			while (true) {
				LexNode element;
				if (lexBracketComment(tempStr, tempBegin, tempEnd, element, tempErrors)) {
					tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
					tempBegin = tempEnd;
					tempNodes.push_back(std::move(element));
					continue;
				}

				if (lexSpace(tempStr, tempBegin, tempEnd, element, tempErrors)) {
					tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
					tempBegin = tempEnd;
					tempNodes.push_back(std::move(element));
					continue;
				}

				break;
			}

			for (auto& error : tempErrors)
				errors.push_back(std::move(error));

			if (!lexLineEnding(tempStr, tempBegin, tempEnd, lineEnding, errors)) {
				errors.emplace_back("Expected lineEnding", tempBegin);
				return false;
			}

			end = tempEnd;
			for (auto& node : tempNodes)
				node.m_Children.push_back(std::move(node));
			node.m_Children.push_back(std::move(lineEnding));
			return true;
		};

		if (!variant1()) {
			if (!variant2()) {
				errors.emplace_back("Expected commandInvocation or lineEnding", begin);
				return false;
			}
		}
		node.m_Type  = LexNodeType::FileElement;
		node.m_Begin = begin;
		node.m_End   = end;
		return true;
	}

	bool lexLineEnding(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// lineComment? newline
		LexNode lineComment;
		LexNode newline;
		SourceRef tempBegin = begin;
		SourceRef tempEnd   = begin;
		std::vector<LexError> tempErrors;

		if (lexLineComment(str, tempBegin, tempEnd, lineComment, tempErrors)) {
			str       = str.substr(tempEnd.m_Index, tempBegin.m_Index);
			tempBegin = tempEnd;
		}

		if (!lexNewline(str, tempBegin, end, newline, errors)) {
			errors.emplace_back("Expected newline", tempBegin);
			return false;
		}
		node.m_Type  = LexNodeType::LineEnding;
		node.m_Begin = begin;
		node.m_End   = end;
		return true;
	}

	bool lexSpace(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// (' ' | '\t')+
		if (str.empty() || (str[0] != ' ' && str[0] != '\t')) {
			errors.emplace_back("Expected space or tab", begin);
			return false;
		}

		std::size_t len = 1;
		while (len < str.size()) {
			bool found = false;
			switch (str[len]) {
			case ' ':
				[[fallthrough]];
			case '\t':
				found = true;
				break;
			}

			if (!found)
				break;
			++len;
		}

		end.m_Index  = begin.m_Index + len;
		end.m_Line   = begin.m_Line;
		end.m_Column = begin.m_Column + len;
		return true;
	}

	bool lexNewline(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '\n'
		if (str.empty() || str[0] != '\n') {
			errors.emplace_back("Expected newline", begin);
			return false;
		}

		end.m_Index  = begin.m_Index + 1;
		end.m_Line   = begin.m_Line + 1;
		end.m_Column = 0;
		return true;
	}

	bool lexCommandInvocation(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// space* identifier space* '\\(' arguments '\\)'
		LexNode space;
		LexNode identifier;
		LexNode arguments;
		SourceRef tempBegin = begin;
		SourceRef tempEnd   = begin;

		while (true) {
			if (!lexSpace(str, tempBegin, tempEnd, space, errors))
				break;

			str       = str.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
		}

		if (!lexIdentifier(str, tempBegin, tempEnd, identifier, errors)) {
			errors.emplace_back("Expected identifier", tempBegin);
			return false;
		}
		node.m_Children.push_back(std::move(identifier));
		str       = str.substr(tempEnd.m_Index - tempBegin.m_Index);
		tempBegin = tempEnd;

		while (true) {
			if (!lexSpace(str, tempBegin, tempEnd, space, errors))
				break;

			str       = str.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
		}

		if (str.empty() || str[0] != '(') {
			errors.emplace_back("Expected '('", tempBegin);
			return false;
		}
		str = str.substr(1);
		++tempBegin.m_Index;
		++tempBegin.m_Column;

		if (!lexArguments(str, tempBegin, tempEnd, arguments, errors)) {
			errors.emplace_back("Expected arguments", tempBegin);
			return false;
		}
		node.m_Children.push_back(std::move(arguments));
		str       = str.substr(tempEnd.m_Index - tempBegin.m_Index);
		tempBegin = tempEnd;

		if (str.empty() || str[0] != ')') {
			errors.emplace_back("Expected ')'", tempBegin);
			return false;
		}
		str = str.substr(1);
		++tempBegin.m_Index;
		++tempBegin.m_Column;

		end          = tempBegin;
		node.m_Type  = LexNodeType::CommandInvocation;
		node.m_Begin = begin;
		node.m_End   = end;
		return true;
	}

	bool lexIdentifier(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '[A-Za-z_][A-Za-z0-9_]*'
		if (str.empty() || (!isalpha(str[0]) && str[0] != '_')) {
			errors.emplace_back("Expected character between 'A'-'Z', 'a'-'z' or '_'", begin);
			return false;
		}

		std::size_t len = 1;
		while (len < str.size() && (isalpha(str[len]) || isdigit(str[len]) || str[len] == '_'))
			++len;

		end.m_Index  = begin.m_Index + len;
		end.m_Line   = begin.m_Line;
		end.m_Column = begin.m_Column + len;

		node.m_Type  = LexNodeType::Identifier;
		node.m_Begin = begin;
		node.m_End   = end;
		node.m_Str   = str.substr(0, end.m_Index - begin.m_Index);
		return true;
	}

	bool lexArguments(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// argument? separatedArguments*
		std::string_view tempStr = str;
		SourceRef tempBegin      = begin;
		SourceRef tempEnd        = begin;
		LexNode argument;
		std::vector<LexError> tempErrors;
		if (lexArgument(tempStr, tempBegin, tempEnd, argument, tempErrors)) {
			tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
			node.m_Children.push_back(std::move(argument));
		}

		while (true) {
			LexNode separatedArgument;
			if (!lexSeparatedArguments(tempStr, tempBegin, tempEnd, separatedArgument, errors))
				break;
			tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
			node.m_Children.push_back(std::move(separatedArgument));
		}

		node.m_Type  = LexNodeType::Arguments;
		node.m_Begin = begin;
		node.m_End   = end;
		return true;
	}

	bool lexSeparatedArguments(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// separation+ argument?
		auto variant1 = [&]() -> bool {
			std::string_view tempStr = str;
			SourceRef tempBegin      = begin;
			SourceRef tempEnd        = begin;
			LexNode separation;
			LexNode tempArgument;
			std::vector<LexError> tempErrors;

			bool hasSeparation = false;
			while (true) {
				if (!lexSeparation(tempStr, tempBegin, tempEnd, separation, tempErrors))
					break;
				hasSeparation = true;
				tempStr       = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
				tempBegin     = tempEnd;
			}

			if (!hasSeparation)
				return false;

			if (!lexArgument(tempStr, tempBegin, tempEnd, tempArgument, tempErrors))
				return false;

			for (auto& error : tempErrors)
				errors.push_back(std::move(error));

			end  = tempEnd;
			node = std::move(tempArgument);
			return true;
		};

		// separation* '\\(' arguments '\\)'
		auto variant2 = [&]() -> bool {
			std::string_view tempStr = str;
			SourceRef tempBegin      = begin;
			SourceRef tempEnd        = begin;
			LexNode separation;
			LexNode arguments;
			std::vector<LexError> tempErrors;

			while (true) {
				if (!lexSeparation(tempStr, tempBegin, tempEnd, separation, tempErrors))
					break;
				tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
				tempBegin = tempEnd;
			}

			for (auto& error : tempErrors)
				errors.push_back(std::move(error));

			if (str.empty() || str[0] != '(') {
				errors.emplace_back("Expected '('", tempBegin);
				return false;
			}
			str = str.substr(1);
			++tempBegin.m_Index;
			++tempBegin.m_Column;

			if (!lexArguments(tempStr, tempBegin, tempEnd, arguments, errors)) {
				errors.emplace_back("Expected arguments", tempBegin);
				return false;
			}
			tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;

			if (str.empty() || str[0] != ')') {
				errors.emplace_back("Expected ')'", tempBegin);
				return false;
			}
			str = str.substr(1);
			++tempBegin.m_Index;
			++tempBegin.m_Column;

			end  = tempEnd;
			node = std::move(arguments);
			return true;
		};

		if (!variant1()) {
			if (!variant2()) {
				errors.emplace_back("Expected argument or '(' arguments ')'", begin);
				return false;
			}
		}
		return true;
	}

	bool lexSeparation(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// space
		if (!lexSpace(str, begin, end, node, errors)) {
			// lineEnding
			if (!lexLineEnding(str, begin, end, node, errors)) {
				errors.emplace_back("Expected space or lineEnding", begin);
				return false;
			}
		}
		return true;
	}

	bool lexArgument(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// bracketArgument
		if (!lexBracketArgument(str, begin, end, node, errors)) {
			// quotedArgument
			if (!lexQuotedArgument(str, begin, end, node, errors)) {
				// unquotedArgument
				if (!lexUnquotedArgument(str, begin, end, node, errors)) {
					errors.emplace_back("Expected bracketArgument, QuotedArgument or UnquotedArgument", begin);
					return false;
				}
			}
		}
		return true;
	}

	bool lexBracketArgument(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// bracketOpen bracketContent bracketClose
		std::string_view tempStr = str;
		SourceRef tempBegin      = begin;
		SourceRef tempEnd        = begin;
		LexNode bracketOpen;
		LexNode bracketContent;
		LexNode bracketClose;

		std::size_t bracketCount = 0;

		if (!lexBracketOpen(tempStr, tempBegin, tempEnd, bracketOpen, errors, bracketCount)) {
			errors.emplace_back("Expected bracketOpen", tempBegin);
			return false;
		}
		tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
		tempBegin = tempEnd;

		if (!lexBracketContent(tempStr, tempBegin, tempEnd, bracketContent, errors, bracketCount)) {
			errors.emplace_back("Expected bracketContent", tempBegin);
			return false;
		}
		tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
		tempBegin = tempEnd;

		if (!lexBracketClose(tempStr, tempBegin, end, bracketClose, errors, bracketCount)) {
			errors.emplace_back("Expected bracketClose", tempBegin);
			return false;
		}

		node = std::move(bracketContent);
		return true;
	}

	bool lexBracketOpen(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors, std::size_t& bracketCount) {
		// '\\[' (<BracketCount>: '='*) '\\['
		if (str.empty() || str[0] != '[') {
			errors.emplace_back("Expected '['", begin);
			return false;
		}

		bracketCount = 0;
		while (1 + bracketCount < str.size() && str[1 + bracketCount] == '=')
			++bracketCount;

		if (2 + bracketCount < str.size() || str[2 + bracketCount] != '[') {
			errors.emplace_back("Expected '=' or '['", SourceRef { begin.m_Index + 2 + bracketCount, begin.m_Line, begin.m_Column + 2 + bracketCount });
			return false;
		}

		end.m_Index  = begin.m_Index + 3 + bracketCount;
		end.m_Line   = begin.m_Line;
		end.m_Column = begin.m_Column + 3 + bracketCount;
		return true;
	}

	bool lexBracketContent(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors, std::size_t bracketCount) {
		// ('.' | newline)*
		std::string_view tempStr = str;
		SourceRef tempBegin      = begin;
		SourceRef tempEnd        = begin;
		SourceRef tempTempEnd;
		LexNode tempNode;
		std::vector<LexError> tempErrors;
		while (true) {
			if (tempStr[0] != '\n') {
				if (tempStr[0] == ']' && lexBracketClose(tempStr, tempBegin, tempTempEnd, tempNode, tempErrors, bracketCount))
					break;

				tempEnd = tempBegin;
				++tempEnd.m_Index;
				++tempEnd.m_Column;
			} else if (!lexNewline(tempStr, tempBegin, tempEnd, tempNode, tempErrors))
				break;
			tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
		}
		end = tempEnd;

		node.m_Type  = LexNodeType::BracketContent;
		node.m_Begin = begin;
		node.m_End   = end;
		node.m_Str   = str.substr(0, end.m_Index - begin.m_Index);
		return true;
	}

	bool lexBracketClose(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors, std::size_t bracketCount) {
		// '\\]' \BracketCount '\\]'
		if (str.empty() || str[0] != ']') {
			errors.emplace_back("Expected ']'", begin);
			return false;
		}

		if (1 + bracketCount < str.size()) {
			errors.emplace_back("Expected '='", SourceRef { begin.m_Index + 1, begin.m_Line, begin.m_Column + 1 });
			return false;
		}

		for (std::size_t i = 0; i < bracketCount; ++i) {
			if (str[1 + i] != '=') {
				errors.emplace_back("Expected '='", SourceRef { begin.m_Index + 1 + i, begin.m_Line, begin.m_Column + 1 + i });
				return false;
			}
		}

		if (2 + bracketCount < str.size() || str[2 + bracketCount] != ']') {
			errors.emplace_back("Expected ']'", SourceRef { begin.m_Index + 2 + bracketCount, begin.m_Line, begin.m_Column + 2 + bracketCount });
			return false;
		}
		end.m_Index  = begin.m_Index + 3 + bracketCount;
		end.m_Line   = begin.m_Line;
		end.m_Column = begin.m_Column + 3 + bracketCount;
		return true;
	}

	bool lexQuotedArgument(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '"' quotedElement* '"'
		std::string_view tempStr = str;
		SourceRef tempBegin      = begin;
		SourceRef tempEnd        = begin;

		if (tempStr.empty() || tempStr[0] != '"') {
			errors.emplace_back("Expected '\"'", tempBegin);
			return false;
		}
		tempStr = tempStr.substr(1);
		++tempBegin.m_Index;
		++tempBegin.m_Column;

		while (true) {
			LexNode quotedElement;
			if (!lexQuotedArgument(tempStr, tempBegin, tempEnd, quotedElement, errors))
				break;
			tempStr   = tempStr.substr(tempEnd.m_Index, tempBegin.m_Index);
			tempBegin = tempEnd;
			node.m_Children.push_back(std::move(quotedElement));
		}

		if (tempStr.empty() || tempStr[0] != '\"') {
			errors.emplace_back("Expected '\"'", tempBegin);
			return false;
		}
		tempStr = tempStr.substr(1);
		++tempBegin.m_Index;
		++tempBegin.m_Column;

		end = tempEnd;

		node.m_Type  = LexNodeType::QuotedArgument;
		node.m_Begin = begin;
		node.m_End   = end;
		node.m_Str   = str.substr(1, end.m_Index - begin.m_Index - 2);
		return true;
	}

	bool lexQuotedElement(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// (newline | '[^"\\]')*
		auto variant1 = [&]() -> bool {
			std::string_view tempStr = str;
			SourceRef tempBegin      = begin;
			SourceRef tempEnd        = begin;
			LexNode tempNode;
			std::vector<LexError> tempErrors;
			while (!tempStr.empty()) {
				if (lexNewline(tempStr, tempBegin, tempEnd, tempNode, tempErrors)) {
					tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
					tempBegin = tempEnd;
					continue;
				}

				if (str[0] == '"' || str[0] == '\\')
					break;

				tempEnd.m_Index  = tempBegin.m_Index + 1;
				tempEnd.m_Line   = tempBegin.m_Line;
				tempEnd.m_Column = tempBegin.m_Column + 1;
				tempStr          = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
				tempBegin        = tempEnd;
			}
			end = tempEnd;

			node.m_Type  = LexNodeType::QuotedElement;
			node.m_Begin = begin;
			node.m_End   = end;
			node.m_Str   = str.substr(0, end.m_Index - begin.m_Index);
			return true;
		};

		std::vector<LexError> tempErrors;
		if (!lexEscapeSequence(str, begin, end, node, tempErrors))
			if (!lexQuotedContinuation(str, begin, end, node, tempErrors))
				variant1();
		return true;
	}

	bool lexQuotedContinuation(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '\\' newline
		std::string_view tempStr = str;
		SourceRef tempBegin      = begin;
		SourceRef tempEnd        = begin;
		LexNode tempNode;
		if (tempStr.empty() || tempStr[0] != '\\') {
			errors.emplace_back("Expected '\\'", tempBegin);
			return false;
		}
		tempStr = tempStr.substr(1);
		++tempBegin.m_Index;
		++tempBegin.m_Column;

		if (!lexNewline(tempStr, tempBegin, tempEnd, tempNode, errors)) {
			errors.emplace_back("Expected newline", tempBegin);
			return false;
		}

		end = tempEnd;
		return true;
	}

	bool lexUnquotedArgument(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// unquotedElement+
		auto variant1 = [&]() -> bool {
			std::string_view tempStr = str;
			SourceRef tempBegin      = begin;
			SourceRef tempEnd        = begin;
			std::vector<LexError> tempErrors;
			bool hasElement = false;

			while (true) {
				LexNode tempNode;
				if (!lexUnquotedElement(tempStr, tempBegin, tempEnd, tempNode, tempErrors))
					break;
				hasElement = true;
				tempStr    = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
				tempBegin  = tempEnd;
				node.m_Children.push_back(std::move(tempNode));
			}

			if (!hasElement) {
				errors.emplace_back("Expected unquotedElement", begin);
				return false;
			}
			end = tempEnd;

			node.m_Type  = LexNodeType::UnquotedArgument;
			node.m_Begin = begin;
			node.m_End   = end;
			return true;
		};

		if (!variant1()) {
			// unqoutedLegacy
			if (!lexUnquotedLegacy(str, begin, end, node, errors)) {
				errors.emplace_back("Expected unquotedLegacy", begin);
				return false;
			}
		}
		return true;
	}

	bool lexUnquotedElement(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '(?:[^\\s()#"\\\\])+'
		auto variant1 = [&]() -> bool {
			std::size_t len = 0;

			while (true) {
				bool breakOut = false;
				switch (str[len]) {
				case ' ':
					[[fallthrough]];
				case '\t':
					[[fallthrough]];
				case '\n':
					[[fallthrough]];
				case '(':
					[[fallthrough]];
				case ')':
					[[fallthrough]];
				case '#':
					[[fallthrough]];
				case '"':
					[[fallthrough]];
				case '\\':
					breakOut = true;
					break;
				}

				if (breakOut)
					break;
				++len;
			}

			if (len == 0) {
				errors.emplace_back("Expected text", begin);
				return false;
			}

			end.m_Index  = begin.m_Index + len;
			end.m_Line   = begin.m_Line;
			end.m_Column = begin.m_Column;

			node.m_Type  = LexNodeType::UnquotedElement;
			node.m_Begin = begin;
			node.m_End   = end;
			node.m_Str   = str.substr(0, end.m_Index - begin.m_Index);
			return true;
		};

		std::vector<LexError> tempErrors;
		// escapeSequence
		if (!lexEscapeSequence(str, begin, end, node, tempErrors)) {
			if (!variant1()) {
				errors.emplace_back("Expected text or escapeSequence", begin);
				return false;
			}
		}
		return true;
	}

	bool lexUnquotedLegacy(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		errors.emplace_back("unqoutedLegacy isn't currently supported!", begin);
		return false;
	}

	bool lexEscapeSequence(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		std::vector<LexError> tempErrors;
		// escapeIdentity
		if (!lexEscapeIdentity(str, begin, end, node, tempErrors)) {
			// escapeEncoded
			if (!lexEscapeEncoded(str, begin, end, node, tempErrors)) {
				// escapeSemicolon
				if (!lexEscapeSemicolon(str, begin, end, node, errors)) {
					errors.emplace_back("Expected escapeIdentity, escapeEncoded or escapeSemicolon");
					return false;
				}
			}
		}
		return true;
	}

	bool lexEscapeIdentity(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '\\[^A-Za-z0-9;]'
		if (str.empty() || str[0] != '\\') {
			errors.emplace_back("Expected '\\'", begin);
			return false;
		}

		if (1 < str.size() && (!std::isalnum(str[1]) && str[1] != ';')) {
			errors.emplace_back("Expected A-Za-z0-9 or ';'", SourceRef { begin.m_Index + 1, begin.m_Line, begin.m_Column + 1 });
			return false;
		}

		end.m_Index  = begin.m_Index + 2;
		end.m_Line   = begin.m_Line;
		end.m_Column = begin.m_Column + 2;

		node.m_Type  = LexNodeType::EscapeIdentity;
		node.m_Begin = begin;
		node.m_End   = end;
		node.m_Str   = str.substr(1, end.m_Index - begin.m_Index - 1);
		return true;
	}

	bool lexEscapeEncoded(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '\\t' | '\\r' | '\\n'
		if (str.empty() || str[0] != '\\') {
			errors.emplace_back("Expected '\\'", begin);
			return false;
		}

		if (1 < str.size() && (str[1] != 't' && str[1] != 'r' && str[1] != 'n')) {
			errors.emplace_back("Expected 't', 'r' or 'n'", SourceRef { begin.m_Index + 1, begin.m_Line, begin.m_Column + 1 });
			return false;
		}

		end.m_Index  = begin.m_Index + 2;
		end.m_Line   = begin.m_Line;
		end.m_Column = begin.m_Column + 2;

		node.m_Type  = LexNodeType::EscapeEncoded;
		node.m_Begin = begin;
		node.m_End   = end;
		node.m_Str   = str.substr(1, end.m_Index - begin.m_Index - 1);
		return true;
	}

	bool lexEscapeSemicolon(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '\\;'
		if (str.empty() || str[0] != '\\') {
			errors.emplace_back("Expected '\\'", begin);
			return false;
		}

		if (1 < str.size() && str[1] != ';') {
			errors.emplace_back("Expected ';'", SourceRef { begin.m_Index + 1, begin.m_Line, begin.m_Column + 1 });
			return false;
		}

		end.m_Index  = begin.m_Index + 2;
		end.m_Line   = begin.m_Line;
		end.m_Column = begin.m_Column + 2;

		node.m_Type  = LexNodeType::EscapeSemicolon;
		node.m_Begin = begin;
		node.m_End   = end;
		node.m_Str   = str.substr(1, end.m_Index - begin.m_Index - 1);
		return true;
	}

	bool lexLineComment(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '#(?!\\[=*\\[).*'
		if (str.empty() || str[0] != '#') {
			errors.emplace_back("Expected '#'", begin);
			return false;
		}

		if (1 < str.size() && str[1] == '[') {
			std::size_t len = 2;
			while (len < str.size() && str[len] == '=')
				++len;
			if (str[len] == '[')
				return false;
		}

		std::size_t len = 1;
		while (len < str.size() && str[len] != '\n')
			++len;

		end.m_Index  = begin.m_Index + len;
		end.m_Line   = begin.m_Line;
		end.m_Column = begin.m_Column;
		return true;
	}

	bool lexBracketComment(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '#' bracketArgument
		std::string_view tempStr = str;
		SourceRef tempBegin      = begin;
		SourceRef tempEnd        = begin;
		LexNode tempNode;

		if (tempStr.empty() || tempStr[0] != '#') {
			errors.emplace_back("Expected '#'", begin);
			return false;
		}
		++tempBegin.m_Index;
		++tempBegin.m_Column;
		tempStr = tempStr.substr(1);

		if (!lexBracketArgument(tempStr, tempBegin, tempEnd, tempNode, errors)) {
			errors.emplace_back("Expected bracketArgument", tempBegin);
			return false;
		}

		return true;
	}
} // namespace CMakeCompiler
