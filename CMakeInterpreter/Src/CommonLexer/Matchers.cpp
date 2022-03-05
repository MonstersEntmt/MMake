#include "CommonLexer/Matchers.h"
#include "CommonLexer/Lexer.h"

#include <format>
#include <iostream>

namespace CommonLexer
{
	CombinationMatcher::CombinationMatcher(std::vector<std::unique_ptr<IMatcher>>&& matchers)
	    : m_Matchers(std::move(matchers)) {}

	void CombinationMatcher::cleanUp()
	{
		for (auto& matcher : m_Matchers)
			matcher->cleanUp();
	}

	MatchResult CombinationMatcher::match(MatcherState& state, SourceSpan span)
	{
		SourceSpan subSpan   = span;
		SourceSpan totalSpan = { span.m_Begin, span.m_Begin };
		for (auto& matcher : m_Matchers)
		{
			auto result = matcher->match(state, subSpan);
			switch (result.m_Status)
			{
			case EMatchStatus::Failure: return { EMatchStatus::Failure, totalSpan };
			case EMatchStatus::Skip: continue;
			case EMatchStatus::Success:
				subSpan.m_Begin = totalSpan.m_End = result.m_Span.m_End;
				break;
			default: return { EMatchStatus::Failure, totalSpan };
			}
		}
		return { EMatchStatus::Success, totalSpan };
	}

	OrMatcher::OrMatcher(std::vector<std::unique_ptr<IMatcher>>&& matchers)
	    : m_Matchers(std::move(matchers)) {}

	void OrMatcher::cleanUp()
	{
		for (auto& matcher : m_Matchers)
			matcher->cleanUp();
	}

	MatchResult OrMatcher::match(MatcherState& state, SourceSpan span)
	{
		EMatchStatus         status = EMatchStatus::Failure;
		std::vector<Message> messages;
		Node                 largestCopy { *state.m_Lexer };
		SourceSpan           largestSpan { span.m_Begin, span.m_Begin };
		for (auto& matcher : m_Matchers)
		{
			Node         tempNode { *state.m_Lexer };
			MatcherState tempState { {}, &tempNode, state.m_Lexer, state.m_Source, state.m_SourceSpan, state.m_CurrentRule, state.m_RuleBegin };
			auto         result = matcher->match(tempState, span);
			switch (result.m_Status)
			{
			case EMatchStatus::Skip:
				if (status == EMatchStatus::Failure)
					status = EMatchStatus::Skip;
				[[fallthrough]];
			case EMatchStatus::Failure:
			{
				std::size_t len = result.m_Span.length();
				if (len == 0)
					len = 1;
				if ((status == EMatchStatus::Skip || status == EMatchStatus::Failure) && len > largestSpan.length())
				{
					largestSpan = result.m_Span;
					largestCopy = std::move(tempNode);
					messages    = std::move(tempState.m_Messages);
				}
				continue;
			}
			case EMatchStatus::Success:
				if (status == EMatchStatus::Skip || status == EMatchStatus::Failure || result.m_Span.length() > largestSpan.length())
				{
					largestSpan = result.m_Span;
					largestCopy = std::move(tempNode);
					messages    = std::move(tempState.m_Messages);
					status      = EMatchStatus::Success;
				}
				break;
			}
		}

		state.m_Messages.reserve(state.m_Messages.size() + messages.size());
		for (auto& message : messages)
			state.m_Messages.push_back(std::move(message));
		switch (status)
		{
		case EMatchStatus::Failure: return { EMatchStatus::Failure, largestSpan };
		case EMatchStatus::Skip: return { EMatchStatus::Skip, largestSpan };
		case EMatchStatus::Success:
			state.m_ParentNode->addChildren(largestCopy);
			return { EMatchStatus::Success, largestSpan };
		default: return { EMatchStatus::Failure, largestSpan };
		}
	}

	RangeMatcher::RangeMatcher(std::unique_ptr<IMatcher>&& matcher, std::size_t lowerBounds, std::size_t upperBounds)
	    : m_Matcher(std::move(matcher)), m_LowerBounds(lowerBounds), m_UpperBounds(upperBounds) {}

	void RangeMatcher::cleanUp()
	{
		m_Matcher->cleanUp();
	}

	MatchResult RangeMatcher::match(MatcherState& state, SourceSpan span)
	{
		std::size_t  matches   = 0;
		SourceSpan   subSpan   = span;
		SourceSpan   totalSpan = { span.m_Begin, span.m_Begin };
		MatcherState tempState = { {}, state.m_ParentNode, state.m_Lexer, state.m_Source, state.m_SourceSpan, state.m_CurrentRule, state.m_RuleBegin };
		while (matches < m_UpperBounds)
		{
			auto result = m_Matcher->match(tempState, subSpan);
			if (result.m_Status != EMatchStatus::Success)
				break;

			subSpan.m_Begin = totalSpan.m_End = result.m_Span.m_End;
			++matches;

			if (subSpan.length() == 0)
				break;
		}

		if (totalSpan.m_End.m_Index < span.m_End.m_Index)
		{
			state.m_Messages.reserve(state.m_Messages.size() + tempState.m_Messages.size());
			for (auto& message : tempState.m_Messages)
				state.m_Messages.push_back(std::move(message));
		}

		if (matches < m_LowerBounds)
		{
			state.m_Messages.emplace_back(std::format("Expected at least {} matches but only got {} matches", m_LowerBounds, matches), subSpan.m_End, subSpan);
			return { EMatchStatus::Failure, totalSpan };
		}
		return { EMatchStatus::Success, totalSpan };
	}

	OptionalMatcher::OptionalMatcher(std::unique_ptr<IMatcher>&& matcher)
	    : m_Matcher(std::move(matcher)) {}

	void OptionalMatcher::cleanUp()
	{
		m_Matcher->cleanUp();
	}

	MatchResult OptionalMatcher::match(MatcherState& state, SourceSpan span)
	{
		auto result = m_Matcher->match(state, span);
		switch (result.m_Status)
		{
		case EMatchStatus::Failure: [[fallthrough]];
		case EMatchStatus::Skip: return { EMatchStatus::Skip, result.m_Span };
		case EMatchStatus::Success: return { EMatchStatus::Success, result.m_Span };
		default: return { EMatchStatus::Skip, result.m_Span };
		}
	}

	NegativeMatcher::NegativeMatcher(std::unique_ptr<IMatcher>&& matcher)
	    : m_Matcher(std::move(matcher)) {}

	void NegativeMatcher::cleanUp()
	{
		m_Matcher->cleanUp();
	}

	MatchResult NegativeMatcher::match(MatcherState& state, SourceSpan span)
	{
		Node         tempNode { *state.m_Lexer };
		MatcherState tempState { {}, &tempNode, state.m_Lexer, state.m_Source, state.m_SourceSpan, state.m_CurrentRule, state.m_RuleBegin };
		auto         result = m_Matcher->match(tempState, span);
		switch (result.m_Status)
		{
		case EMatchStatus::Failure: [[fallthrough]];
		case EMatchStatus::Skip: return { EMatchStatus::Success, { span.m_Begin, span.m_Begin } };
		case EMatchStatus::Success:
			state.m_Messages.emplace_back("Did not expect following sequence", span.m_Begin, result.m_Span);
			return { EMatchStatus::Failure, result.m_Span };
		default: return { EMatchStatus::Failure, result.m_Span };
		}
	}

	SpaceMatcher::SpaceMatcher(std::unique_ptr<IMatcher>&& matcher, bool forced, ESpaceMethod method, ESpaceDirection direction)
	    : m_Matcher(std::move(matcher)), m_Forced(forced), m_Method(method), m_Direction(direction) {}

	void SpaceMatcher::cleanUp()
	{
		m_Matcher->cleanUp();
	}

	MatchResult SpaceMatcher::match(MatcherState& state, SourceSpan span)
	{
		MatchResult result { EMatchStatus::Failure, { span.m_Begin, span.m_Begin } };
		SourceSpan  subSpan   = span;
		SourceSpan  totalSpan = { span.m_Begin, span.m_Begin };
		if (m_Direction == ESpaceDirection::Left || m_Direction == ESpaceDirection::Both)
		{
			result = matchSpaces(state, subSpan);
			switch (result.m_Status)
			{
			case EMatchStatus::Failure: return { EMatchStatus::Failure, totalSpan };
			case EMatchStatus::Skip: break;
			case EMatchStatus::Success:
				subSpan.m_Begin = totalSpan.m_End = result.m_Span.m_End;
				break;
			default: return { EMatchStatus::Failure, totalSpan };
			}
		}

		result = m_Matcher->match(state, subSpan);
		switch (result.m_Status)
		{
		case EMatchStatus::Failure: return { EMatchStatus::Failure, totalSpan };
		case EMatchStatus::Skip: break;
		case EMatchStatus::Success:
			subSpan.m_Begin = totalSpan.m_End = result.m_Span.m_End;
			break;
		default: return { EMatchStatus::Failure, totalSpan };
		}

		if (m_Direction == ESpaceDirection::Right || m_Direction == ESpaceDirection::Both)
		{
			result = matchSpaces(state, subSpan);
			switch (result.m_Status)
			{
			case EMatchStatus::Failure: return { EMatchStatus::Failure, totalSpan };
			case EMatchStatus::Skip: break;
			case EMatchStatus::Success:
				subSpan.m_Begin = totalSpan.m_End = result.m_Span.m_End;
				break;
			default: return { EMatchStatus::Failure, totalSpan };
			}
		}

		return { EMatchStatus::Success, totalSpan };
	}

	bool SpaceMatcher::isSpace(char c)
	{
		switch (m_Method)
		{
		case ESpaceMethod::Normal:
			switch (c)
			{
			case '\t': [[fallthrough]];
			case ' ':
				return true;
			default:
				return false;
			}
			break;
		case ESpaceMethod::Whitespace:
			switch (c)
			{
			case '\t': [[fallthrough]];
			case '\n': [[fallthrough]];
			case '\v': [[fallthrough]];
			case '\f': [[fallthrough]];
			case '\r': [[fallthrough]];
			case ' ':
				return true;
			default:
				return false;
			}
			break;
		default:
			return false;
		}
	}

	MatchResult SpaceMatcher::matchSpaces(MatcherState& state, SourceSpan span)
	{
		std::size_t i = 0;

		auto begin = state.m_SourceSpan.begin(state.m_Source);
		auto itr   = span.begin(state.m_Source);
		auto end   = span.end(state.m_Source);
		if (m_Forced)
		{
			--itr;
			if (itr >= begin && isSpace(*itr))
				++i;
			++itr;
		}
		while (itr != end)
		{
			if (!isSpace(*itr))
				break;
			++itr;
			++i;
		}

		if (m_Forced && i == 0)
		{
			switch (m_Method)
			{
			case ESpaceMethod::Normal:
				state.m_Messages.emplace_back(std::format("Expected space or tab but got '{}'", itr != end ? std::string { *itr } : "EOF"), span.m_Begin, SourceSpan { span.m_Begin, span.m_Begin });
				break;
			case ESpaceMethod::Whitespace:
				state.m_Messages.emplace_back(std::format("Expected space, tab, vertical tab, form feed, carriage return or line feed but got '{}'", itr != end ? std::string { *itr } : "EOF"), span.m_Begin, SourceSpan { span.m_Begin, span.m_Begin });
				break;
			}
			return { EMatchStatus::Failure, { span.m_Begin, span.m_Begin } };
		}
		return { EMatchStatus::Success, { span.m_Begin, itr } };
	}

	NamedGroupMatcher::NamedGroupMatcher(const std::string& name, std::unique_ptr<IMatcher>&& matcher)
	    : m_Name(name), m_Matcher(std::move(matcher)) {}

	NamedGroupMatcher::NamedGroupMatcher(std::string&& name, std::unique_ptr<IMatcher>&& matcher)
	    : m_Name(std::move(name)), m_Matcher(std::move(matcher)) {}

	void NamedGroupMatcher::cleanUp()
	{
		m_Matcher->cleanUp();
	}

	MatchResult NamedGroupMatcher::match(MatcherState& state, SourceSpan span)
	{
		auto result = m_Matcher->match(state, span);
		switch (result.m_Status)
		{
		case EMatchStatus::Failure: return { EMatchStatus::Failure, result.m_Span };
		case EMatchStatus::Skip: return { EMatchStatus::Skip, result.m_Span };
		case EMatchStatus::Success:
			state.m_Lexer->setGroupedValue(m_Name, result.m_Span);
			return { EMatchStatus::Success, result.m_Span };
		default: return { EMatchStatus::Failure, result.m_Span };
		}
	}

	NamedGroupReferenceMatcher::NamedGroupReferenceMatcher(const std::string& name)
	    : m_Name(name) {}

	NamedGroupReferenceMatcher::NamedGroupReferenceMatcher(std::string&& name)
	    : m_Name(std::move(name)) {}

	MatchResult NamedGroupReferenceMatcher::match(MatcherState& state, SourceSpan span)
	{
		auto groupedSpan = state.m_Lexer->getGroupedValue(m_Name);

		auto itr        = span.begin(state.m_Source);
		auto end        = span.end(state.m_Source);
		auto groupedItr = groupedSpan.begin(state.m_Source);
		auto groupedEnd = groupedSpan.end(state.m_Source);

		while (groupedItr != groupedEnd)
		{
			if (itr == end)
			{
				state.m_Messages.emplace_back(std::format("Expected '{}' but got 'EOF', {}", state.m_Source->getSpan({ groupedItr, groupedEnd }), state.m_CurrentRule->getName()), itr, SourceSpan { span.m_Begin, itr });
				return { EMatchStatus::Failure, { span.m_Begin, itr } };
			}

			if (*itr != *groupedItr)
			{
				state.m_Messages.emplace_back(std::format("Expected '{}' but got '{}', {}", *groupedItr, *itr, state.m_CurrentRule->getName()), itr, SourceSpan { span.m_Begin, itr });
				return { EMatchStatus::Failure, { span.m_Begin, itr } };
			}

			++itr;
			++groupedItr;
		}
		return { EMatchStatus::Success, { span.m_Begin, itr } };
	}

	ReferenceMatcher::ReferenceMatcher(const std::string& name)
	    : m_Name(name), m_Rule(nullptr) {}

	ReferenceMatcher::ReferenceMatcher(std::string&& name)
	    : m_Name(std::move(name)), m_Rule(nullptr) {}

	void ReferenceMatcher::cleanUp()
	{
		m_Rule = nullptr;
	}

	MatchResult ReferenceMatcher::match(MatcherState& state, SourceSpan span)
	{
		if (!m_Rule)
		{
			m_Rule = state.m_Lexer->getRule(m_Name);
			if (!m_Rule)
			{
				state.m_Messages.emplace_back(std::format("Expected non existent rule '{}'", m_Name), span.m_Begin, SourceSpan { span.m_Begin, span.m_Begin });
				return { EMatchStatus::Failure, { span.m_Begin, span.m_Begin } };
			}
		}

		return m_Rule->match(state, span);
	}

	TextMatcher::TextMatcher(const std::string& text)
	    : m_Text(text) {}

	TextMatcher::TextMatcher(std::string&& text)
	    : m_Text(std::move(text)) {}

	MatchResult TextMatcher::match(MatcherState& state, SourceSpan span)
	{
		auto itr     = span.begin(state.m_Source);
		auto end     = span.end(state.m_Source);
		auto textItr = m_Text.begin();
		auto textEnd = m_Text.end();

		while (textItr != textEnd)
		{
			if (itr == end)
			{
				state.m_Messages.emplace_back(std::format("Expected '{}' but got 'EOF', {}", std::string_view { textItr, textEnd }, state.m_CurrentRule->getName()), itr, SourceSpan { span.m_Begin, itr });
				return { EMatchStatus::Failure, { span.m_Begin, itr } };
			}

			if (*itr != *textItr)
			{
				state.m_Messages.emplace_back(std::format("Expected '{}' but got '{}', {}", *textItr, *itr, state.m_CurrentRule->getName()), itr, SourceSpan { state.m_RuleBegin, itr });
				return { EMatchStatus::Failure, { span.m_Begin, itr } };
			}

			++itr;
			++textItr;
		}
		return { EMatchStatus::Success, { span.m_Begin, itr } };
	}

	RegexMatcher::RegexMatcher(const std::string& regex)
	    : m_Regex(regex, std::regex_constants::ECMAScript | std::regex_constants::optimize) {}

	RegexMatcher::RegexMatcher(std::string&& regex)
	    : m_Regex(std::move(regex), std::regex_constants::ECMAScript | std::regex_constants::optimize) {}

	MatchResult RegexMatcher::match(MatcherState& state, SourceSpan span)
	{
		std::match_results<SourceIterator> results;
		if (std::regex_search(span.begin(state.m_Source), span.end(state.m_Source), results, m_Regex, std::regex_constants::match_continuous))
			return { EMatchStatus::Success, { results[0].first, results[0].second } };
		state.m_Messages.emplace_back(std::format("Expected regex to succeed, but failed. Sadly I don't get regex error messages, maybe in the future ;), {}", state.m_CurrentRule->getName()), span.m_Begin, SourceSpan { span.m_Begin, span.m_Begin });
		return { EMatchStatus::Failure, { span.m_Begin, span.m_Begin } };
	}
} // namespace CommonLexer