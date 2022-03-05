#pragma once

#include "Matcher.h"
#include "Rule.h"
#include "Tuple.h"

#include <memory>
#include <regex>

namespace CommonLexer
{
	//---------
	// Depth 2
	//---------

	struct CombinationMatcher final : public IMatcher
	{
	public:
		template <Matcher... Matchers>
		CombinationMatcher(Tuple<Matchers...>&& matchers);
		CombinationMatcher(std::vector<std::unique_ptr<IMatcher>>&& matchers);

		virtual void        cleanUp() override;
		virtual MatchResult match(MatcherState& state, SourceSpan span) override;

	private:
		std::vector<std::unique_ptr<IMatcher>> m_Matchers;
	};

	struct OrMatcher final : public IMatcher
	{
	public:
		template <Matcher... Matchers>
		OrMatcher(Tuple<Matchers...>&& matchers);
		OrMatcher(std::vector<std::unique_ptr<IMatcher>>&& matchers);

		virtual void        cleanUp() override;
		virtual MatchResult match(MatcherState& state, SourceSpan span) override;

	private:
		std::vector<std::unique_ptr<IMatcher>> m_Matchers;
	};

	//---------
	// Depth 3
	//---------

	struct RangeMatcher final : public IMatcher
	{
	public:
		template <Matcher Matcher>
		RangeMatcher(Matcher&& matcher, std::size_t lowerBounds = 0, std::size_t upperBounds = ~0ULL);
		RangeMatcher(std::unique_ptr<IMatcher>&& matcher, std::size_t lowerBounds = 0, std::size_t upperBounds = ~0ULL);

		virtual void        cleanUp() override;
		virtual MatchResult match(MatcherState& state, SourceSpan span) override;

	private:
		std::unique_ptr<IMatcher> m_Matcher;
		std::size_t               m_LowerBounds, m_UpperBounds;
	};

	struct OptionalMatcher final : public IMatcher
	{
	public:
		template <Matcher Matcher>
		OptionalMatcher(Matcher&& matcher);
		OptionalMatcher(std::unique_ptr<IMatcher>&& matcher);

		virtual void        cleanUp() override;
		virtual MatchResult match(MatcherState& state, SourceSpan span) override;

	private:
		std::unique_ptr<IMatcher> m_Matcher;
	};

	struct NegativeMatcher final : public IMatcher
	{
	public:
		template <Matcher Matcher>
		NegativeMatcher(Matcher&& matcher);
		NegativeMatcher(std::unique_ptr<IMatcher>&& matcher);

		virtual void        cleanUp() override;
		virtual MatchResult match(MatcherState& state, SourceSpan span) override;

	private:
		std::unique_ptr<IMatcher> m_Matcher;
	};

	//---------
	// Depth 4
	//---------

	enum class ESpaceDirection
	{
		Right,
		Left,
		Both
	};

	enum class ESpaceMethod
	{
		Normal,
		Whitespace
	};

	struct SpaceMatcher final : public IMatcher
	{
	public:
		template <Matcher Matcher>
		SpaceMatcher(Matcher&& matcher, bool forced = false, ESpaceMethod method = ESpaceMethod::Normal, ESpaceDirection direction = ESpaceDirection::Right);
		SpaceMatcher(std::unique_ptr<IMatcher>&& matcher, bool forced = false, ESpaceMethod method = ESpaceMethod::Normal, ESpaceDirection direction = ESpaceDirection::Right);

		virtual void        cleanUp() override;
		virtual MatchResult match(MatcherState& state, SourceSpan span) override;

		bool        isSpace(char c);
		MatchResult matchSpaces(MatcherState& state, SourceSpan span);

	private:
		std::unique_ptr<IMatcher> m_Matcher;
		ESpaceDirection           m_Direction;
		ESpaceMethod              m_Method;
		bool                      m_Forced;
	};

	//---------
	// Depth 5
	//---------

	struct NamedGroupMatcher final : public IMatcher
	{
	public:
		template <Matcher Matcher>
		NamedGroupMatcher(const std::string& name, Matcher&& matcher);
		template <Matcher Matcher>
		NamedGroupMatcher(std::string&& name, Matcher&& matcher);
		NamedGroupMatcher(const std::string& name, std::unique_ptr<IMatcher>&& matcher);
		NamedGroupMatcher(std::string&& name, std::unique_ptr<IMatcher>&& matcher);

		virtual void        cleanUp() override;
		virtual MatchResult match(MatcherState& state, SourceSpan span) override;

	private:
		std::string               m_Name;
		std::unique_ptr<IMatcher> m_Matcher;
	};

	struct NamedGroupReferenceMatcher final : public IMatcher
	{
	public:
		NamedGroupReferenceMatcher(const std::string& name);
		NamedGroupReferenceMatcher(std::string&& name);

		virtual void        cleanUp() override {}
		virtual MatchResult match(MatcherState& state, SourceSpan span) override;

	private:
		std::string m_Name;
	};

	struct ReferenceMatcher final : public IMatcher
	{
	public:
		ReferenceMatcher(const std::string& name);
		ReferenceMatcher(std::string&& name);

		virtual void        cleanUp() override;
		virtual MatchResult match(MatcherState& state, SourceSpan span) override;

	private:
		std::string m_Name;

		IRule* m_Rule;
	};

	struct TextMatcher final : public IMatcher
	{
	public:
		TextMatcher(const std::string& text);
		TextMatcher(std::string&& text);

		virtual void        cleanUp() override {}
		virtual MatchResult match(MatcherState& state, SourceSpan span) override;

	private:
		std::string m_Text;
	};

	struct RegexMatcher final : public IMatcher
	{
	public:
		RegexMatcher(const std::string& regex);
		RegexMatcher(std::string&& regex);

		virtual void        cleanUp() override {}
		virtual MatchResult match(MatcherState& state, SourceSpan span) override;

	private:
		std::regex m_Regex;
	};

	template <Matcher... Matchers>
	CombinationMatcher::CombinationMatcher(Tuple<Matchers...>&& matchers)
	    : m_Matchers(Details::make_unique_ptrs<IMatcher>(std::move(matchers)))
	{
	}

	template <Matcher... Matchers>
	OrMatcher::OrMatcher(Tuple<Matchers...>&& matchers)
	    : m_Matchers(Details::make_unique_ptrs<IMatcher>(std::move(matchers)))
	{
	}

	template <Matcher Matcher>
	RangeMatcher::RangeMatcher(Matcher&& matcher, std::size_t lowerBounds, std::size_t upperBounds)
	    : m_Matcher(std::make_unique<Matcher>(std::move(matcher))), m_LowerBounds(lowerBounds), m_UpperBounds(upperBounds)
	{
	}

	template <Matcher Matcher>
	OptionalMatcher::OptionalMatcher(Matcher&& matcher)
	    : m_Matcher(std::make_unique<Matcher>(std::move(matcher)))
	{
	}

	template <Matcher Matcher>
	NegativeMatcher::NegativeMatcher(Matcher&& matcher)
	    : m_Matcher(std::make_unique<Matcher>(std::move(matcher)))
	{
	}

	template <Matcher Matcher>
	SpaceMatcher::SpaceMatcher(Matcher&& matcher, bool forced, ESpaceMethod method, ESpaceDirection direction)
	    : m_Matcher(std::make_unique<Matcher>(std::move(matcher))), m_Forced(forced), m_Method(method), m_Direction(direction)
	{
	}

	template <Matcher Matcher>
	NamedGroupMatcher::NamedGroupMatcher(const std::string& name, Matcher&& matcher)
	    : m_Name(name), m_Matcher(std::make_unique<Matcher>(std::move(matcher)))
	{
	}

	template <Matcher Matcher>
	NamedGroupMatcher::NamedGroupMatcher(std::string&& name, Matcher&& matcher)
	    : m_Name(std::move(name)), m_Matcher(std::make_unique<Matcher>(std::move(matcher)))
	{
	}
} // namespace CommonLexer