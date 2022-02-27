#pragma once

#include "CommonLexer/Lexer.h"

namespace CMakeLexer
{
	class Lexer : public CommonLexer::Lexer
	{
	public:
		Lexer();

		CommonLexer::LexResult unquotedLegacyCallback(CommonLexer::Lex& lex, CommonLexer::LexNode& parentNode, CommonLexer::ISource* source, const CommonLexer::SourceSpan& span);
	};
} // namespace CMakeLexer
