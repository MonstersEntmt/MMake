#pragma once

#include <cstddef>
#include <cstdint>

#include <vector>
#include <string>
#include <string_view>

namespace CMakeCompiler {
	struct SourceRef {
	public:
		std::size_t m_Index  = 0;
		std::size_t m_Line   = 1;
		std::size_t m_Column = 1;
	};
	
	enum class LexNodeType {
		File,
		FileElement,
		LineEnding,
		Space,
		Newline,
		CommandInvocation,
		Identifier,
		Arguments,
		SeparatedArguments,
		Separation,
		Argument,
		BracketArgument,
		BracketOpen,
		BracketContent,
		BracketClose,
		QuotedArgument,
		QuotedElement,
		QuotedContinuation,
		UnquotedArgument,
		UnquotedElement,
		UnquotedLegacy,
		EscapeSequence,
		EscapeIdentity,
		EscapeEncoded,
		EscapeSemicolon,
		LineComment,
		BracketComment
	};
	
	struct LexNode {
	public:
		LexNodeType m_Type;
		SourceRef m_Begin;
		SourceRef m_End;
		std::string_view m_Str;
		std::vector<LexNode> m_Children;
	};
	
	struct Lex {
	public:
		LexNode m_Root;
	};
	
	struct LexError {
	public:
		std::string m_Message;
		SourceRef m_At;
	};
	
	bool lexFile              (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexFileElement       (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexLineEnding        (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexSpace             (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexNewline           (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexCommandInvocation (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexIdentifier        (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexArguments         (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexSeparatedArguments(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexSeparation        (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexArgument          (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexBracketArgument   (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexBracketOpen       (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexBracketContent    (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexBracketClose      (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexQuotedArgument    (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexQuotedElement     (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexQuotedContinuation(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexUnquotedArgument  (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexUnquotedElement   (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexUnquotedLegacy    (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexEscapeSequence    (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexEscapeIdentity    (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexEscapeEncoded     (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexEscapeSemicolon   (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexLineComment       (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	bool lexBracketComment    (std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	
	Lex lexString(std::string_view str, SourceRef begin, std::vector<LexError>& errors) {
		Lex lex;
		lexFile(str, begin, lex, errors);
		return lex;
	}
}
