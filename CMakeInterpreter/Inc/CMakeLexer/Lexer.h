#pragma once

#include "CommonLexer/Lexer.h"

namespace CMakeLexer {
	class Lexer : public CommonLexer::Lexer {
	public:
		Lexer();

		virtual CommonLexer::Lex lexSource(CommonLexer::ISource* source) override;
		
		auto* getTFile() const { return &m_TFile; }
		auto* getTFileElement() const { return &m_TFileElement; }
		auto* getTCommandInvocation() const { return &m_TCommandInvocation; }
		auto* getTIdentifier() const { return &m_TIdentifier; }
		auto* getTArguments() const { return &m_TArguments; }
		auto* getTBracketContent() const { return &m_TBracketContent; }
		auto* getTQuotedArgument() const { return &m_TQuotedArgument; }
		auto* getTQuotedElement() const { return &m_TQuotedElement; }
		auto* getTUnquotedArgument() const { return &m_TUnquotedArgument; }
		auto* getTUnquotedElement() const { return &m_TUnquotedElement; }
		auto* getTUnquotedLegacy() const { return &m_TUnquotedLegacy; }
		auto* getTEscapeSequence() const { return &m_TEscapeSequence; }
		auto* getTLineComment() const { return &m_TLineComment; }
		auto* getTBracketComment() const { return &m_TBracketComment; }

	private:
		CommonLexer::LexNodeType m_TFile;
		CommonLexer::LexNodeType m_TFileElement;
		CommonLexer::LexNodeType m_TCommandInvocation;
		CommonLexer::LexNodeType m_TIdentifier;
		CommonLexer::LexNodeType m_TArguments;
		CommonLexer::LexNodeType m_TBracketContent;
		CommonLexer::LexNodeType m_TQuotedArgument;
		CommonLexer::LexNodeType m_TQuotedElement;
		CommonLexer::LexNodeType m_TUnquotedArgument;
		CommonLexer::LexNodeType m_TUnquotedElement;
		CommonLexer::LexNodeType m_TUnquotedLegacy;
		CommonLexer::LexNodeType m_TEscapeSequence;
		CommonLexer::LexNodeType m_TLineComment;
		CommonLexer::LexNodeType m_TBracketComment;
	};
} // namespace CMakeLexer
