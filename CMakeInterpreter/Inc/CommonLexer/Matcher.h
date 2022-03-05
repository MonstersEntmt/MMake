#pragma once

#include "Message.h"
#include "Node.h"
#include "Source.h"

#include <vector>

namespace CommonLexer
{
	class Lexer;
	struct IRule;

	struct MatcherState
	{
	public:
		std::vector<Message> m_Messages;
		Node*                m_ParentNode;

		Lexer* m_Lexer;

		ISource*   m_Source;
		SourceSpan m_SourceSpan;

		IRule*      m_CurrentRule;
		SourcePoint m_RuleBegin;
	};

	enum class EMatchStatus
	{
		Success,
		Skip,
		Failure
	};

	struct MatchResult
	{
	public:
		MatchResult(EMatchStatus status, SourceSpan span);

		EMatchStatus m_Status;
		SourceSpan   m_Span;
	};

	struct IMatcher
	{
	public:
		virtual ~IMatcher() = default;

		virtual void        cleanUp()                                   = 0;
		virtual MatchResult match(MatcherState& state, SourceSpan span) = 0;
	};

	template <class T>
	concept Matcher = std::is_base_of_v<IMatcher, T>;
} // namespace CommonLexer