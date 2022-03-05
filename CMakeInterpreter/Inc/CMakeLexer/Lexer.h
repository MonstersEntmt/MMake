#pragma once

#include "CommonLexer/Lexer.h"

namespace CMakeLexer
{
	class Lexer : public CommonLexer::Lexer
	{
	public:
		Lexer();

		CommonLexer::MatchResult unquotedLegacyCallback(CommonLexer::MatcherState& state, CommonLexer::SourceSpan span);
	};
} // namespace CMakeLexer
