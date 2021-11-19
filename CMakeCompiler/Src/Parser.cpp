#include "CMakeCompiler/Parser.h"

#include <utility>

namespace CMakeCompiler {
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
		node.m_Type = LexNodeType::File;
		node.m_Begin = begin;
		node.m_End = end;
		return true;
	}
	
	bool lexFileElement(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		// commandInvocation lineEnding
		auto variant1 = [&]() -> bool {
			std::string_view tempStr = str;
			SourceRef tempEnd;
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
			SourceRef tempBegin = begin;
			SourceRef tempEnd;
			std::vector<LexNode> tempNodes;
			LexNode lineEnding;
			std::vector<LexError> tempErrors;
			
			while (true) {
				LexNode element;
				if (lexBracketComment(tempStr, tempBegin, tempEnd, element, tempErrors)) {
					tempStr = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
					tempBegin = tempEnd;
					tempNodespush_back(std::move(element));
					continue;
				}
				
				if (lexSpace(tempStr, tempBegin, tempEnd, element, tempErrors)) {
					tempStr = tempStr.substr(tempEnd.m_Index - tempBegin.m_Index);
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
		node.m_Type = LexNodeType::FileElement;
		node.m_Begin = begin;
		node.m_End = end;
		return true;
	}
	
	bool lexLineEnding(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		LexNode lineComment;
		LexNode newline;
		SourceRef tempBegin = begin;
		SourceRef tempEnd;
		std::vector<LexError> tempErrors;
		
		if (lexLineComment(str, tempBegin, tempEnd, lineComment, tempErrors)) {
			str = str.substr(tempEnd.m_Index, tempBegin.m_Index);
			tempBegin = tempEnd;
		}
		
		if (!lexNewline(str, tempBegin, end, newline, errors)) {
			errors.emplace_back("Expected newline", tempBegin);
			return false;
		}
		node.m_Type = LexNodeType::LineEnding;
		node.m_Begin = begin;
		node.m_End = end;
		return true;
	}
	
	bool lexSpace(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		if (str.empty() || (str[0] != ' ' && str[0] != '\t')) {
			errors.emplace_back("Expected space or tab", begin);
			return false;
		}
		
		std::size_t len = 1;
		while (true) {
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
		if (str.empty() || str[0] != '\n') {
			errors.emplace_back("Expected newline", begin);
			return false;
		}
		
		end.m_Index = begin.m_Index + 1;
		end.m_Line = begin.m_Line + 1;
		end.m_Column = 0;
		return true;
	}
	
	bool lexCommandInvocation(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		LexNode space;
		LexNode identifier;
		LexNode arguments;
		SourceRef tempBegin = begin;
		SourceRef tempEnd;
		
		while (true) {
			if (!lexSpace(str, tempBegin, tempEnd, space, errors))
				break;
			
			str = str.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
		}
		
		if (!lexIdentifier(str, tempBegin, tempEnd, identifier, errors)) {
			errors.emplace_back("Expected identifier", tempBegin);
			return false;
		}
		node.m_Children.push_back(std::move(identifier));
		str = str.substr(tempEnd.m_Index - tempBegin.m_Index);
		tempBegin = tempEnd;
		
		while (true) {
			if (!lexSpace(str, tempBegin, tempEnd, space, errors))
				break;
			
			str = str.substr(tempEnd.m_Index - tempBegin.m_Index);
			tempBegin = tempEnd;
		}
		
		if (str.empty() || str[0] != '(') {
			errors.emplace_back("Expected '('", tempBegin);
			return false;
		}
		++tempBegin.m_Index;
		str = str.substr(1);
		
		if (!lexArguments(str, tempBegin, tempEnd, arguments, errors)) {
			errors.emplace_back("Expected arguments", tempBegin);
			return false;
		}
		node.m_Children.push_back(std::move(arguments));
		str = str.substr(tempEnd.m_Index - tempBegin.m_Index);
		tempBegin = tempEnd;
		
		if (str.empty() || str[0] != ')') {
			errors.emplace_back("Expected ')'", tempBegin);
			return false;
		}
		++tempBegin.m_Index;
		str = str.substr(1);
		end = tempBegin;
		node.m_Type = LexNodeType::CommandInvocation;
		node.m_Begin = begin;
		node.m_End = end;
		return true;
	}
	
	bool lexIdentifier(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
		node.m_Type = LexNodeType::Identifier;
		node.m_Begin = begin;
		node.m_End = end;
		node.m_Str = str.substr(end.m_Index - begin.m_Index);
		return true;
	}
	
	bool lexArguments(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexSeparatedArguments(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexSeparation(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexArgument(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexBracketArgument(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexBracketOpen(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexBracketContent(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexBracketClose(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexQuotedArgument(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexQuotedElement(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexQuotedContinuation(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexUnquotedArgument(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexUnquotedElement(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexUnquotedLegacy(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexEscapeSequence(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexEscapeIdentity(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexEscapeEncoded(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexEscapeSemicolon(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexLineComment(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
	
	bool lexBracketComment(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors) {
		
	}
}
