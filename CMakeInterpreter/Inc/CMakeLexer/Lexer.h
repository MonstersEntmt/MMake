#pragma once

#include "CommonLexer/Lexer.h"

namespace CMakeLexer {
	class Lexer : public CommonLexer::Lexer {
	public:
		Lexer();

		virtual CommonLexer::Lex lexSource(CommonLexer::ISource* source) override;

	private:
		CommonLexer::LexNodeType m_TFile;
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