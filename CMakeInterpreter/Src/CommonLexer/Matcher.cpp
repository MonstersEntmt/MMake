#include "CommonLexer/Matcher.h"

namespace CommonLexer
{
	MatchResult::MatchResult(EMatchStatus status, SourceSpan span)
	    : m_Status(status), m_Span(span) {}
} // namespace CommonLexer