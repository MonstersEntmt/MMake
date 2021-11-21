#include "CMakeCompiler/Lexer.h"

#include <utility>

namespace CMakeCompiler {
	SourceRef::SourceRef(std::size_t index, std::size_t line, std::size_t column)
	    : m_Index(index), m_Line(line), m_Column(column) { }

	LexError::LexError(const std::string& message, const SourceRef& at)
	    : m_Message(message), m_At(at) { }

	LexError::LexError(std::string&& message, SourceRef&& at)
	    : m_Message(std::move(message)), m_At(std::move(at)) { }

	LexResult lexFile(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// fileElement*
		std::string_view tempStr = str;
		SourceRef tempBegin      = begin;
		SourceRef tempEnd        = begin;
		std::vector<LexError> tempErrors;

		while (true) {
			tempErrors.clear();
			LexNode elementNode;
			auto result = lexFileElement(tempStr, tempBegin, tempEnd, elementNode, tempErrors);
			if (result == LexResult::Error) {
				for (auto& error : tempErrors)
					errors.push_back(std::move(error));
				errors.emplace_back("Broken fileElement", tempBegin);
				return LexResult::Error;
			} else if (result == LexResult::Done) {
				break;
			}
			tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
			if (elementNode.m_Type != LexNodeType::Unknown)
				node.m_Children.push_back(std::move(elementNode));
		}
		end = tempEnd;

		node.m_Type  = LexNodeType::File;
		node.m_Begin = begin;
		node.m_End   = end;
		return LexResult::Success;
	}

	LexResult lexFileElement(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// commandInvocation lineEnding
		auto variant1 = [&]() -> LexResult {
			std::string_view tempStr = str;
			SourceRef tempBegin      = begin;
			SourceRef tempEnd        = begin;
			LexNode commandInvocation;
			LexNode lineEnding;
			std::vector<LexError> tempErrors;

			auto result = lexCommandInvocation(tempStr, tempBegin, tempEnd, commandInvocation, tempErrors);
			if (result == LexResult::Error) {
				for (auto& error : tempErrors)
					errors.push_back(std::move(error));
				errors.emplace_back("Broken commandInvocation", tempBegin);
				return LexResult::Error;
			} else if (result == LexResult::Skip) {
				return LexResult::Skip;
			}
			tempErrors.clear();
			tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;

			result = lexLineEnding(tempStr, tempBegin, tempEnd, lineEnding, errors);
			if (result == LexResult::Error) {
				for (auto& error : tempErrors)
					errors.push_back(std::move(error));
				errors.emplace_back("Broken lineEnding", tempBegin);
				return LexResult::Error;
			} else if (result == LexResult::Skip) {
				for (auto& error : tempErrors)
					errors.push_back(std::move(error));
				errors.emplace_back("Expected lineEnding", tempBegin);
				return LexResult::Error;
			}

			end = tempEnd;
			node.m_Children.push_back(std::move(commandInvocation));
			return LexResult::Success;
		};

		// (bracketComment | space)* lineEnding
		auto variant2 = [&]() -> LexResult {
			std::string_view tempStr = str;
			SourceRef tempBegin      = begin;
			SourceRef tempEnd        = begin;
			LexNode lineEnding;
			std::vector<LexError> tempErrors;

			LexNode tempNode;
			while (true) {
				auto result = lexBracketComment(tempStr, tempBegin, tempEnd, tempNode, tempErrors);
				if (result == LexResult::Success) {
					tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
					tempBegin = tempEnd;
					continue;
				} else if (result == LexResult::Error) {
					for (auto& error : tempErrors)
						errors.push_back(std::move(error));
					errors.emplace_back("Broken bracketComment", begin);
					return LexResult::Error;
				}
				tempErrors.clear();

				result = lexSpace(tempStr, tempBegin, tempEnd, tempNode, tempErrors);
				if (result == LexResult::Success) {
					tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
					tempBegin = tempEnd;
					continue;
				} else if (result == LexResult::Error) {
					for (auto& error : tempErrors)
						errors.push_back(std::move(error));
					errors.emplace_back("Broken bracketComment", begin);
					return LexResult::Error;
				}
				break;
			}
			tempErrors.clear();

			auto result = lexLineEnding(tempStr, tempBegin, tempEnd, lineEnding, tempErrors);
			if (result == LexResult::Error) {
				for (auto& error : tempErrors)
					errors.push_back(std::move(error));
				errors.emplace_back("Broken lineEnding", tempBegin);
				return LexResult::Error;
			} else if (result == LexResult::Skip) {
				return LexResult::Done;
			}

			end = tempEnd;
			return LexResult::Success;
		};

		auto result = variant1();
		if (result == LexResult::Success) {
			node.m_Type  = LexNodeType::FileElement;
			node.m_Begin = begin;
			node.m_End   = end;
			return LexResult::Success;
		} else if (result == LexResult::Error) {
			return LexResult::Error;
		}

		result = variant2();
		if (result == LexResult::Success) {
			return LexResult::Skip;
		} else if (result == LexResult::Error) {
			return LexResult::Error;
		} else if (result == LexResult::Done) {
			return LexResult::Done;
		}
		return LexResult::Success;
	}

	LexResult lexLineEnding(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// lineComment? newline
		std::string_view tempStr = str;
		SourceRef tempBegin      = begin;
		SourceRef tempEnd        = begin;
		LexNode lineComment;
		LexNode newline;
		std::vector<LexError> tempErrors;

		bool hasLineComment = false;

		auto result = lexLineComment(tempStr, tempBegin, tempEnd, lineComment, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken lineComment", tempBegin);
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			tempStr        = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin      = tempEnd;
			hasLineComment = true;
		}

		result = lexNewline(tempStr, tempBegin, end, newline, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken newline", tempBegin);
			return LexResult::Error;
		} else if (result == LexResult::Skip) {
			if (hasLineComment) {
				errors.emplace_back("Expected newline", tempBegin);
				return LexResult::Error;
			}
			return LexResult::Skip;
		}
		return LexResult::Success;
	}

	LexResult lexSpace(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// (' ' | '\t')+
		if (str.empty() || (str[0] != ' ' && str[0] != '\t'))
			return LexResult::Skip;

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
		return LexResult::Success;
	}

	LexResult lexNewline(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '\n'
		if (str.empty() || str[0] != '\n')
			return LexResult::Skip;

		end.m_Index  = begin.m_Index + 1;
		end.m_Line   = begin.m_Line + 1;
		end.m_Column = 1;
		return LexResult::Success;
	}

	LexResult lexCommandInvocation(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// space* identifier space* '\\(' arguments '\\)'
		std::string_view tempStr = str;
		SourceRef tempBegin      = begin;
		SourceRef tempEnd        = begin;
		LexNode space;
		LexNode identifier;
		LexNode arguments;
		std::vector<LexError> tempErrors;

		LexResult result;

		while (true) {
			result = lexSpace(tempStr, tempBegin, tempEnd, space, tempErrors);
			if (result == LexResult::Error) {
				for (auto& error : tempErrors)
					errors.push_back(std::move(error));
				errors.emplace_back("Broken space", tempBegin);
				return LexResult::Error;
			} else if (result == LexResult::Skip) {
				break;
			}

			tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
		}
		tempErrors.clear();

		result = lexIdentifier(tempStr, tempBegin, tempEnd, identifier, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken identifier", tempBegin);
			return LexResult::Error;
		} else if (result == LexResult::Skip) {
			errors.emplace_back("Expected identifier", tempBegin);
			return LexResult::Skip;
		} else if (result == LexResult::Success) {
			node.m_Children.push_back(std::move(identifier));
			tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
		}
		tempErrors.clear();

		while (true) {
			result = lexSpace(tempStr, tempBegin, tempEnd, space, tempErrors);
			if (result == LexResult::Error) {
				for (auto& error : tempErrors)
					errors.push_back(std::move(error));
				errors.emplace_back("Broken space", tempBegin);
				return LexResult::Error;
			} else if (result == LexResult::Skip) {
				break;
			}

			tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
		}
		tempErrors.clear();

		if (tempStr.empty() || tempStr[0] != '(') {
			errors.emplace_back("Expected '('", tempBegin);
			return LexResult::Error;
		}
		tempStr = tempStr.substr(1);
		++tempEnd.m_Index;
		++tempEnd.m_Column;
		tempBegin = tempEnd;

		result = lexArguments(tempStr, tempBegin, tempEnd, arguments, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken arguments", tempBegin);
			return LexResult::Error;
		} else if (result == LexResult::Skip) {
			errors.emplace_back("Expected arguments", tempBegin);
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			node.m_Children.push_back(std::move(arguments));
			tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
		}
		tempErrors.clear();

		if (tempStr.empty() || tempStr[0] != ')') {
			errors.emplace_back("Expected ')'", tempBegin);
			return LexResult::Error;
		}
		tempStr = tempStr.substr(1);
		++tempEnd.m_Index;
		++tempEnd.m_Column;
		tempBegin = tempEnd;

		end          = tempBegin;
		node.m_Type  = LexNodeType::CommandInvocation;
		node.m_Begin = begin;
		node.m_End   = end;
		return LexResult::Success;
	}

	LexResult lexIdentifier(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '[A-Za-z_][A-Za-z0-9_]*'
		if (str.empty() || (!isalpha(str[0]) && str[0] != '_'))
			return LexResult::Skip;

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
		return LexResult::Success;
	}

	LexResult lexArguments(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// argument? separatedArguments*
		std::string_view tempStr = str;
		SourceRef tempBegin      = begin;
		SourceRef tempEnd        = begin;
		LexNode argument;
		std::vector<LexError> tempErrors;

		LexResult result;
		result = lexArgument(tempStr, tempBegin, tempEnd, argument, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken argument", tempBegin);
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
			node.m_Children.push_back(std::move(argument));
		}
		tempErrors.clear();

		while (true) {
			LexNode separatedArgument;
			result = lexSeparatedArguments(tempStr, tempBegin, tempEnd, separatedArgument, tempErrors);
			if (result == LexResult::Error) {
				for (auto& error : tempErrors)
					errors.push_back(std::move(error));
				errors.emplace_back("Broken separatedArguments", tempBegin);
				return LexResult::Error;
			} else if (result == LexResult::Skip) {
				break;
			}
			tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
			node.m_Children.push_back(std::move(separatedArgument));
		}

		end = tempEnd;

		node.m_Type  = LexNodeType::Arguments;
		node.m_Begin = begin;
		node.m_End   = end;
		return LexResult::Success;
	}

	LexResult lexSeparatedArguments(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// separation+ argument?
		auto variant1 = [&]() -> LexResult {
			std::string_view tempStr = str;
			SourceRef tempBegin      = begin;
			SourceRef tempEnd        = begin;
			LexNode separation;
			LexNode tempArgument;
			std::vector<LexError> tempErrors;

			LexResult result;

			bool hasSeparation = false;
			while (true) {
				result = lexSeparation(tempStr, tempBegin, tempEnd, separation, tempErrors);
				if (result == LexResult::Error) {
					for (auto& error : tempErrors)
						errors.push_back(std::move(error));
					errors.emplace_back("Broken separation", tempBegin);
					return LexResult::Error;
				} else if (result == LexResult::Skip) {
					break;
				}
				tempErrors.clear();

				hasSeparation = true;
				tempStr       = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
				tempBegin     = tempEnd;
			}

			if (!hasSeparation)
				return LexResult::Skip;

			result = lexArgument(tempStr, tempBegin, tempEnd, tempArgument, tempErrors);
			if (result == LexResult::Error) {
				for (auto& error : tempErrors)
					errors.push_back(std::move(error));
				errors.emplace_back("Broken argument", tempBegin);
				return LexResult::Error;
			} else if (result == LexResult::Success) {
				node = std::move(tempArgument);
			}

			end = tempEnd;
			return LexResult::Success;
		};

		// separation* '\\(' arguments '\\)'
		auto variant2 = [&]() -> LexResult {
			std::string_view tempStr = str;
			SourceRef tempBegin      = begin;
			SourceRef tempEnd        = begin;
			LexNode separation;
			LexNode arguments;
			std::vector<LexError> tempErrors;

			LexResult result;

			bool hasSeparation = false;

			while (true) {
				result = lexSeparation(tempStr, tempBegin, tempEnd, separation, tempErrors);
				if (result == LexResult::Error) {
					for (auto& error : tempErrors)
						errors.push_back(std::move(error));
					errors.emplace_back("Broken separation", tempBegin);
					return LexResult::Error;
				} else if (result == LexResult::Skip) {
					break;
				}
				tempErrors.clear();

				hasSeparation = true;
				tempStr       = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
				tempBegin     = tempEnd;
			}

			if (tempStr.empty() || tempStr[0] != '(') {
				if (hasSeparation) {
					errors.emplace_back("Expected '('", tempBegin);
					return LexResult::Error;
				}
				return LexResult::Skip;
			}
			tempStr = tempStr.substr(1);
			++tempEnd.m_Index;
			++tempEnd.m_Column;
			tempBegin = tempEnd;

			result = lexArguments(tempStr, tempBegin, tempEnd, arguments, tempErrors);
			if (result == LexResult::Error) {
				for (auto& error : tempErrors)
					errors.push_back(std::move(error));
				errors.emplace_back("Broken arguments", tempBegin);
				return LexResult::Error;
			} else if (result == LexResult::Skip) {
				errors.emplace_back("Expected arguments", tempBegin);
				return LexResult::Error;
			} else if (result == LexResult::Success) {
				tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
				tempBegin = tempEnd;
				node      = std::move(arguments);
			}
			tempErrors.clear();

			if (tempStr.empty() || tempStr[0] != ')') {
				errors.emplace_back("Expected ')'", tempBegin);
				return LexResult::Error;
			}
			tempStr = tempStr.substr(1);
			++tempEnd.m_Index;
			++tempEnd.m_Column;

			end = tempEnd;
			return LexResult::Success;
		};

		auto result = variant1();
		if (result == LexResult::Error)
			return LexResult::Error;
		else if (result == LexResult::Success)
			return LexResult::Success;

		result = variant2();
		if (result == LexResult::Error)
			return LexResult::Error;
		else if (result == LexResult::Success)
			return LexResult::Success;

		return LexResult::Skip;
	}

	LexResult lexSeparation(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		std::vector<LexError> tempErrors;

		// space
		auto result = lexSpace(str, begin, end, node, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken space", begin);
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			return LexResult::Success;
		}
		tempErrors.clear();

		// lineEnding
		result = lexLineEnding(str, begin, end, node, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken lineEnding", begin);
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			return LexResult::Success;
		}

		return LexResult::Skip;
	}

	LexResult lexArgument(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		std::vector<LexError> tempErrors;

		// bracketArgument
		auto result = lexBracketArgument(str, begin, end, node, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken bracketArgument", begin);
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			return LexResult::Success;
		}
		tempErrors.clear();

		result = lexQuotedArgument(str, begin, end, node, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken quotedArgument", begin);
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			return LexResult::Success;
		}
		tempErrors.clear();

		result = lexUnquotedArgument(str, begin, end, node, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken UnquotedArgument", begin);
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			return LexResult::Success;
		}

		return LexResult::Skip;
	}

	LexResult lexBracketArgument(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// bracketOpen bracketContent bracketClose
		std::string_view tempStr = str;
		SourceRef tempBegin      = begin;
		SourceRef tempEnd        = begin;
		LexNode bracketOpen;
		LexNode bracketContent;
		LexNode bracketClose;
		std::vector<LexError> tempErrors;

		std::size_t bracketCount = 0;

		auto result = lexBracketOpen(tempStr, tempBegin, tempEnd, bracketOpen, tempErrors, bracketCount);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken bracketOpen", tempBegin);
			return LexResult::Error;
		} else if (result == LexResult::Skip) {
			return LexResult::Skip;
		} else if (result == LexResult::Success) {
			tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
		}
		tempErrors.clear();

		result = lexBracketContent(tempStr, tempBegin, tempEnd, bracketContent, tempErrors, bracketCount);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken bracketContent", tempBegin);
			return LexResult::Error;
		} else if (result == LexResult::Skip) {
			errors.emplace_back("Expected bracketContent", tempBegin);
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
		}
		tempErrors.clear();

		result = lexBracketClose(tempStr, tempBegin, tempEnd, bracketContent, tempErrors, bracketCount);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken bracketClose", tempBegin);
			return LexResult::Error;
		} else if (result == LexResult::Skip) {
			errors.emplace_back("Expected bracketClose", tempBegin);
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
		}

		end = tempEnd;

		node = std::move(bracketContent);
		return LexResult::Success;
	}

	LexResult lexBracketOpen(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors, std::size_t& bracketCount) {
		// '\\[' (<BracketCount>: '='*) '\\['
		if (str.empty() || str[0] != '[')
			return LexResult::Skip;

		bracketCount = 0;
		while (1 + bracketCount < str.size() && str[1 + bracketCount] == '=')
			++bracketCount;

		if (1 + bracketCount < str.size() && str[1 + bracketCount] != '[') {
			errors.emplace_back("Expected '=' or '['", SourceRef { begin.m_Index + 2 + bracketCount, begin.m_Line, begin.m_Column + 2 + bracketCount });
			return LexResult::Error;
		}

		end.m_Index  = begin.m_Index + 2 + bracketCount;
		end.m_Line   = begin.m_Line;
		end.m_Column = begin.m_Column + 2 + bracketCount;
		return LexResult::Success;
	}

	LexResult lexBracketContent(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors, std::size_t bracketCount) {
		// ('.' | newline)*
		std::string_view tempStr = str;
		SourceRef tempBegin      = begin;
		SourceRef tempEnd        = begin;
		SourceRef tempTempEnd;
		LexNode tempNode;
		std::vector<LexError> tempErrors;

		LexResult result;

		while (true) {
			if (tempStr[0] != '\n') {
				if (tempStr[0] == ']') {
					result = lexBracketClose(tempStr, tempBegin, tempTempEnd, tempNode, tempErrors, bracketCount);
					tempErrors.clear();
					if (result == LexResult::Success)
						break;
				}

				++tempEnd.m_Index;
				++tempEnd.m_Column;
			} else {
				result = lexNewline(tempStr, tempBegin, tempEnd, tempNode, tempErrors);
				if (result == LexResult::Error) {
					for (auto& error : tempErrors)
						errors.push_back(std::move(error));
					errors.emplace_back("Broken newline", tempBegin);
					return LexResult::Error;
				} else if (result == LexResult::Skip) {
				}
			}
			tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
		}
		end = tempEnd;

		node.m_Type  = LexNodeType::BracketContent;
		node.m_Begin = begin;
		node.m_End   = end;
		node.m_Str   = str.substr(0, end.m_Index - begin.m_Index);
		return LexResult::Success;
	}

	LexResult lexBracketClose(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors, std::size_t bracketCount) {
		// '\\]' \BracketCount '\\]'
		if (str.empty() || str[0] != ']')
			return LexResult::Skip;

		if (str.size() < 1 + bracketCount) {
			errors.emplace_back("Expected " + std::to_string(bracketCount) + " '='", SourceRef { begin.m_Index + 1, begin.m_Line, begin.m_Column + 1 });
			return LexResult::Error;
		}

		for (std::size_t i = 0; i < bracketCount; ++i) {
			if (str[1 + i] != '=') {
				errors.emplace_back("Expected '='", SourceRef { begin.m_Index + 1 + i, begin.m_Line, begin.m_Column + 1 + i });
				return LexResult::Error;
			}
		}

		if (1 + bracketCount < str.size() && str[1 + bracketCount] != ']') {
			errors.emplace_back("Expected ']'", SourceRef { begin.m_Index + 2 + bracketCount, begin.m_Line, begin.m_Column + 2 + bracketCount });
			return LexResult::Error;
		}

		end.m_Index  = begin.m_Index + 2 + bracketCount;
		end.m_Line   = begin.m_Line;
		end.m_Column = begin.m_Column + 2 + bracketCount;
		return LexResult::Success;
	}

	LexResult lexQuotedArgument(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '"' quotedElement* '"'
		std::string_view tempStr = str;
		SourceRef tempBegin      = begin;
		SourceRef tempEnd        = begin;
		std::vector<LexError> tempErrors;

		if (tempStr.empty() || tempStr[0] != '"')
			return LexResult::Skip;

		tempStr = tempStr.substr(1);
		++tempEnd.m_Index;
		++tempEnd.m_Column;
		tempBegin = tempEnd;

		while (true) {
			LexNode quotedElement;
			auto result = lexQuotedElement(tempStr, tempBegin, tempEnd, quotedElement, tempErrors);
			if (result == LexResult::Error) {
				for (auto& error : tempErrors)
					errors.push_back(std::move(error));
				errors.emplace_back("Broken quotedElement", tempBegin);
				return LexResult::Error;
			} else if (result == LexResult::Skip) {
				break;
			} else if (result == LexResult::Success) {
				tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
				tempBegin = tempEnd;
				if (quotedElement.m_Type != LexNodeType::Unknown)
					node.m_Children.push_back(std::move(quotedElement));
			}
			tempErrors.clear();
		}

		if (tempStr.empty() || tempStr[0] != '\"') {
			errors.emplace_back("Expected '\"'", tempBegin);
			return LexResult::Error;
		}

		tempStr = tempStr.substr(1);
		++tempEnd.m_Index;
		++tempEnd.m_Column;
		tempBegin = tempEnd;

		end = tempBegin;

		node.m_Type  = LexNodeType::QuotedArgument;
		node.m_Begin = begin;
		node.m_End   = end;
		return LexResult::Success;
	}

	LexResult lexQuotedElement(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// (newline | '[^"\\]')*
		auto variant1 = [&]() -> LexResult {
			std::string_view tempStr = str;
			SourceRef tempBegin      = begin;
			SourceRef tempEnd        = begin;
			LexNode tempNode;
			std::vector<LexError> tempErrors;

			while (!tempStr.empty()) {
				auto result = lexNewline(tempStr, tempBegin, tempEnd, tempNode, tempErrors);
				if (result == LexResult::Error) {
					for (auto& error : tempErrors)
						errors.push_back(std::move(error));
					errors.emplace_back("Broken newline", tempBegin);
					return LexResult::Error;
				} else if (result == LexResult::Success) {
					tempErrors.clear();
					tempStr   = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
					tempBegin = tempEnd;
					continue;
				}
				tempErrors.clear();

				if (tempStr[0] == '"' || tempStr[0] == '\\')
					break;

				tempEnd.m_Index  = tempBegin.m_Index + 1;
				tempEnd.m_Line   = tempBegin.m_Line;
				tempEnd.m_Column = tempBegin.m_Column + 1;
				tempStr          = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
				tempBegin        = tempEnd;
			}

			if (begin.m_Index == tempEnd.m_Index)
				return LexResult::Skip;

			end = tempEnd;

			node.m_Type  = LexNodeType::QuotedElement;
			node.m_Begin = begin;
			node.m_End   = end;
			node.m_Str   = str.substr(0, end.m_Index - begin.m_Index);
			return LexResult::Success;
		};

		std::vector<LexError> tempErrors;

		auto result = lexEscapeSequence(str, begin, end, node, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken escapeSequence", begin);
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			return LexResult::Success;
		}
		tempErrors.clear();

		result = lexQuotedContinuation(str, begin, end, node, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken quotedContinuation", begin);
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			return LexResult::Success;
		}
		tempErrors.clear();

		result = variant1();
		if (result == LexResult::Error) {
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			return LexResult::Success;
		}
		return LexResult::Skip;
	}

	LexResult lexQuotedContinuation(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '\\' newline
		std::string_view tempStr = str;
		SourceRef tempBegin      = begin;
		SourceRef tempEnd        = begin;
		LexNode tempNode;
		std::vector<LexError> tempErrors;

		if (tempStr.empty() || tempStr[0] != '\\')
			return LexResult::Skip;

		tempStr = tempStr.substr(1);
		++tempEnd.m_Index;
		++tempEnd.m_Column;
		tempBegin = tempEnd;

		auto result = lexNewline(tempStr, tempBegin, tempEnd, tempNode, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken newline", tempBegin);
			return LexResult::Error;
		} else if (result == LexResult::Skip) {
			errors.emplace_back("Expected newline", tempBegin);
			return LexResult::Error;
		}

		end = tempEnd;
		return LexResult::Success;
	}

	LexResult lexUnquotedArgument(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// unquotedElement+
		auto variant1 = [&]() -> LexResult {
			std::string_view tempStr = str;
			SourceRef tempBegin      = begin;
			SourceRef tempEnd        = begin;
			std::vector<LexError> tempErrors;
			LexNode outNode;

			bool hasElement = false;

			while (true) {
				LexNode tempNode;
				auto result = lexUnquotedElement(tempStr, tempBegin, tempEnd, tempNode, tempErrors);
				if (result == LexResult::Error) {
					for (auto& error : tempErrors)
						errors.push_back(std::move(error));
					errors.emplace_back("Broken unquotedElement", tempBegin);
					return LexResult::Error;
				} else if (result == LexResult::Skip) {
					break;
				} else if (result == LexResult::Success) {
					hasElement = true;
					tempStr    = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
					tempBegin  = tempEnd;
					if (tempNode.m_Type != LexNodeType::Unknown)
						outNode.m_Children.push_back(std::move(tempNode));
				}
				tempErrors.clear();
			}

			if (!hasElement)
				return LexResult::Skip;

			if (tempStr[0] != ')') {
				SourceRef fakeTempEnd = tempEnd;
				LexNode tempNode;
				auto result = lexSeparation(tempStr, tempBegin, fakeTempEnd, tempNode, tempErrors);
				if (result == LexResult::Skip)
					return LexResult::Skip;
			}

			end = tempEnd;

			outNode.m_Type  = LexNodeType::UnquotedArgument;
			outNode.m_Begin = begin;
			outNode.m_End   = end;

			node = std::move(outNode);
			return LexResult::Success;
		};

		std::vector<LexError> tempErrors;

		auto result = variant1();
		if (result == LexResult::Error) {
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			return LexResult::Success;
		}

		result = lexUnquotedLegacy(str, begin, end, node, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken unquotedLegacy", begin);
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			return LexResult::Success;
		}

		return LexResult::Skip;
	}

	LexResult lexUnquotedElement(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '(?:[^\\s()#"\\\\])+'
		auto variant1 = [&]() -> LexResult {
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

			if (len == 0)
				return LexResult::Skip;

			end.m_Index  = begin.m_Index + len;
			end.m_Line   = begin.m_Line;
			end.m_Column = begin.m_Column;

			node.m_Type  = LexNodeType::UnquotedElement;
			node.m_Begin = begin;
			node.m_End   = end;
			node.m_Str   = str.substr(0, end.m_Index - begin.m_Index);
			return LexResult::Success;
		};

		std::vector<LexError> tempErrors;

		// escapeSequence
		auto result = lexEscapeSequence(str, begin, end, node, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken escapeSequence", begin);
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			return LexResult::Success;
		}

		result = variant1();
		if (result == LexResult::Error) {
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			return LexResult::Success;
		}

		return LexResult::Skip;
	}

	LexResult lexUnquotedLegacy(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		if (str.empty() || str[0] == '"')
			return LexResult::Skip;

		bool insideString = false;

		bool escaped    = false;
		bool breakOut   = false;
		std::size_t len = 0;
		while (len < str.size()) {
			switch (str[len]) {
			case '\\':
				escaped = true;
				++len;
				break;
			case '"':
				if (!escaped)
					insideString = !insideString;
				++len;
				break;
			case '$':
				++len;
				if (len >= str.size())
					break;

				if (!escaped) {
					if (str[len] == '(') {
						++len;
						while (len < str.size()) {
							if (str[len] == ')')
								break;
							++len;
						}
						++len;
					}
				}
				break;
			case ')':
				[[fallthrough]];
			case '\n':
				breakOut = true;
				break;
			case ' ':
				if (!escaped && !insideString) {
					breakOut = true;
					break;
				}
				++len;
				break;
			default:
				escaped = false;
				++len;
			}

			if (breakOut)
				break;
		}
		end.m_Index  = begin.m_Index + len;
		end.m_Line   = begin.m_Line;
		end.m_Column = begin.m_Column + len;

		node.m_Type  = LexNodeType::UnquotedLegacy;
		node.m_Begin = begin;
		node.m_End   = end;
		node.m_Str   = str.substr(0, end.m_Index - begin.m_Index);
		return LexResult::Success;
	}

	LexResult lexEscapeSequence(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		std::vector<LexError> tempErrors;

		auto result = lexEscapeEncoded(str, begin, end, node, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken escapeEncoded", begin);
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			return LexResult::Success;
		}
		tempErrors.clear();

		result = lexEscapeSemicolon(str, begin, end, node, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken escapeSemicolon", begin);
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			return LexResult::Success;
		}

		result = lexEscapeIdentity(str, begin, end, node, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken escapeIdentity", begin);
			return LexResult::Error;
		} else if (result == LexResult::Success) {
			return LexResult::Success;
		}
		tempErrors.clear();

		return LexResult::Skip;
	}

	LexResult lexEscapeIdentity(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '\\[^A-Za-z0-9;]'
		if (str.empty() || str[0] != '\\')
			return LexResult::Skip;

		if (str.size() < 2 || std::isalnum(str[1]) || str[1] == ';') {
			errors.emplace_back("Expected any character other than A-Za-z0-9 or ';'", SourceRef { begin.m_Index + 1, begin.m_Line, begin.m_Column + 1 });
			return LexResult::Skip;
		}

		end.m_Index  = begin.m_Index + 2;
		end.m_Line   = begin.m_Line;
		end.m_Column = begin.m_Column + 2;

		node.m_Type  = LexNodeType::EscapeIdentity;
		node.m_Begin = begin;
		node.m_End   = end;
		node.m_Str   = str.substr(1, end.m_Index - begin.m_Index - 1);
		return LexResult::Success;
	}

	LexResult lexEscapeEncoded(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '\\t' | '\\r' | '\\n'
		if (str.empty() || str[0] != '\\')
			return LexResult::Skip;

		if (str.size() < 2 || (str[1] != 't' && str[1] != 'r' && str[1] != 'n')) {
			errors.emplace_back("Expected 't', 'r' or 'n'", SourceRef { begin.m_Index + 1, begin.m_Line, begin.m_Column + 1 });
			return LexResult::Skip;
		}

		end.m_Index  = begin.m_Index + 2;
		end.m_Line   = begin.m_Line;
		end.m_Column = begin.m_Column + 2;

		node.m_Type  = LexNodeType::EscapeEncoded;
		node.m_Begin = begin;
		node.m_End   = end;
		node.m_Str   = str.substr(1, end.m_Index - begin.m_Index - 1);
		return LexResult::Success;
	}

	LexResult lexEscapeSemicolon(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '\\;'
		if (str.empty() || str[0] != '\\')
			return LexResult::Skip;

		if (str.size() < 2 || str[1] != ';') {
			errors.emplace_back("Expected ';'", SourceRef { begin.m_Index + 1, begin.m_Line, begin.m_Column + 1 });
			return LexResult::Skip;
		}

		end.m_Index  = begin.m_Index + 2;
		end.m_Line   = begin.m_Line;
		end.m_Column = begin.m_Column + 2;

		node.m_Type  = LexNodeType::EscapeSemicolon;
		node.m_Begin = begin;
		node.m_End   = end;
		node.m_Str   = str.substr(1, end.m_Index - begin.m_Index - 1);
		return LexResult::Success;
	}

	LexResult lexLineComment(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '#(?!\\[=*\\[).*'
		if (str.empty() || str[0] != '#')
			return LexResult::Skip;

		if (1 < str.size() && str[1] == '[') {
			std::size_t len = 2;
			while (len < str.size() && str[len] == '=')
				++len;
			if (str[len] == '[')
				return LexResult::Skip;
		}

		std::size_t len = 1;
		while (len < str.size() && str[len] != '\n')
			++len;

		end.m_Index  = begin.m_Index + len;
		end.m_Line   = begin.m_Line;
		end.m_Column = begin.m_Column;
		return LexResult::Success;
	}

	LexResult lexBracketComment(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// '#' bracketArgument
		std::string_view tempStr = str;
		SourceRef tempBegin      = begin;
		SourceRef tempEnd        = begin;
		LexNode tempNode;
		std::vector<LexError> tempErrors;

		if (tempStr.empty() || tempStr[0] != '#')
			return LexResult::Skip;

		tempStr = tempStr.substr(1);
		++tempEnd.m_Index;
		++tempEnd.m_Column;
		tempBegin = tempEnd;

		auto result = lexBracketArgument(tempStr, tempBegin, tempEnd, tempNode, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken bracketArgument", tempBegin);
			return LexResult::Error;
		} else if (result == LexResult::Skip) {
			return LexResult::Skip;
		}

		return LexResult::Success;
	}
} // namespace CMakeCompiler
