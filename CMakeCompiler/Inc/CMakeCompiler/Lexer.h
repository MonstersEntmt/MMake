#pragma once

#include <cstddef>
#include <cstdint>

#include <string>
#include <string_view>
#include <vector>

namespace CMakeCompiler {
	struct SourceRef {
	public:
		SourceRef() = default;
		SourceRef(std::size_t index, std::size_t line, std::size_t column);
		SourceRef(const SourceRef&) = default;
		SourceRef(SourceRef&&)      = default;
		SourceRef& operator=(const SourceRef&) = default;
		SourceRef& operator=(SourceRef&&) = default;

		std::size_t m_Index  = 0;
		std::size_t m_Line   = 1;
		std::size_t m_Column = 1;
	};

	enum class LexNodeType {
		Unknown,
		File,
		FileElement,
		CommandInvocation,
		Identifier,
		Arguments,
		BracketContent,
		QuotedArgument,
		QuotedElement,
		UnquotedArgument,
		UnquotedElement,
		UnquotedLegacy,
		EscapeIdentity,
		EscapeEncoded,
		EscapeSemicolon
	};

	struct LexNode {
	public:
		LexNodeType m_Type = LexNodeType::Unknown;
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
		LexError() = default;
		LexError(const std::string& message, const SourceRef& at);
		LexError(std::string&& message, SourceRef&& at);
		LexError(const LexError&) = default;
		LexError(LexError&&)      = default;

		std::string m_Message;
		SourceRef m_At;
	};

	enum class LexResult {
		Success,
		Done,
		Skip,
		Error
	};

	LexResult lexFile(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexFileElement(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexLineEnding(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexSpace(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexNewline(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexCommandInvocation(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexIdentifier(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexArguments(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexSeparatedArguments(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexSeparation(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexArgument(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexBracketArgument(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexBracketOpen(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors, std::size_t& bracketCount);
	LexResult lexBracketContent(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors, std::size_t bracketCount);
	LexResult lexBracketClose(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors, std::size_t bracketCount);
	LexResult lexQuotedArgument(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexQuotedElement(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexQuotedContinuation(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexUnquotedArgument(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexUnquotedElement(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexUnquotedLegacy(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexEscapeSequence(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexEscapeIdentity(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexEscapeEncoded(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexEscapeSemicolon(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexLineComment(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);
	LexResult lexBracketComment(std::string_view str, SourceRef begin, SourceRef& end, LexNode& node, std::vector<LexError>& errors);

	inline Lex lexString(std::string_view str, SourceRef begin, SourceRef& end, std::vector<LexError>& errors) {
		Lex lex;
		std::vector<LexError> tempErrors;
		auto result = lexFile(str, begin, end, lex.m_Root, tempErrors);
		if (result == LexResult::Error) {
			for (auto& error : tempErrors)
				errors.push_back(std::move(error));
			errors.emplace_back("Broken file", begin);
		}
		return lex;
	}
} // namespace CMakeCompiler
