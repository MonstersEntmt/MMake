#include "CommonLexer/Lexer.h"

#include <utility>

namespace CommonLexer
{
	Lex::Lex(Lexer& lexer)
	    : m_Lexer(&lexer), m_Source(nullptr), m_Root(lexer) {}

	Lex::Lex(Lexer& lexer, ISource* source)
	    : m_Lexer(&lexer), m_Source(source), m_Root(lexer) {}

	void Lex::setSource(ISource* source)
	{
		m_Source = source;
	}

	void Lex::setMessages(std::vector<Message>&& messages)
	{
		m_Messages = std::move(messages);
	}

	Lex Lexer::lexSource(ISource* source)
	{
		return lexSource(source, source->getCompleteSpan());
	}

	Lex Lexer::lexSource(ISource* source, SourceSpan span)
	{
		m_GroupedValues.clear();

		Lex  lex { *this, source };
		auto rule = getRule(m_MainRule);
		if (rule && source)
		{
			auto& root = lex.getRoot();
			root.setRule("Root");

			MatcherState state { {}, &root, this, source, span, nullptr, span.m_Begin };

			auto result = rule->match(state, span);
			if (result.m_Status == EMatchStatus::Success)
				root.setSourceSpan(source, result.m_Span);
			else
				root.setSourceSpan(source, { span.m_Begin, span.m_Begin });
			lex.setMessages(std::move(state.m_Messages));
		}
		return lex;
	}

	void Lexer::registerRule(std::unique_ptr<IRule>&& rule)
	{
		m_Rules.push_back(std::move(rule));
	}

	IRule* Lexer::getRule(const std::string& rule)
	{
		for (auto& rl : m_Rules)
			if (rl->getName() == rule)
				return rl.get();
		return nullptr;
	}

	void Lexer::setMainRule(const std::string& mainRule)
	{
		m_MainRule = mainRule;
	}

	void Lexer::setGroupedValue(const std::string& group, SourceSpan span)
	{
		m_GroupedValues.insert_or_assign(group, span);
	}

	void Lexer::setGroupedValue(std::string&& group, SourceSpan span)
	{
		m_GroupedValues.insert_or_assign(std::move(group), span);
	}

	SourceSpan Lexer::getGroupedValue(const std::string& group) const
	{
		auto itr = m_GroupedValues.find(group);
		return itr != m_GroupedValues.end() ? itr->second : SourceSpan {};
	}
} // namespace CommonLexer