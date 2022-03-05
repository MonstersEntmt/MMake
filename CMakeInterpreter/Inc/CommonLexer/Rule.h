#pragma once

#include "Matcher.h"

#include <functional>
#include <memory>

namespace CommonLexer
{
	struct IRule : public IMatcher
	{
	public:
		IRule(const std::string& name, bool createNode = true);
		IRule(std::string&& name, bool createNode = true);

		[[nodiscard]] auto& getName() const { return m_Name; }
		[[nodiscard]] auto  doesCreateNode() const { return m_CreateNode; }

	protected:
		std::string m_Name;
		bool        m_CreateNode;
	};

	template <class T>
	concept Rule = std::is_base_of_v<IRule, T>;

	struct MatcherRule final : public IRule
	{
	public:
		template <Matcher Matcher>
		MatcherRule(const std::string& name, Matcher&& matcher, bool createNode = true);
		template <Matcher Matcher>
		MatcherRule(std::string&& name, Matcher&& matcher, bool createNode = true);
		MatcherRule(const std::string& name, std::unique_ptr<IMatcher>&& matcher, bool createNode = true);
		MatcherRule(std::string&& name, std::unique_ptr<IMatcher>&& matcher, bool createNode = true);

		virtual void         cleanUp() override;
		virtual MatchResult  match(MatcherState& state, SourceSpan span) override;

	private:
		std::unique_ptr<IMatcher> m_Matcher;
	};

	struct CallbackRule final : public IRule
	{
	public:
		using Callback = std::function<MatchResult(MatcherState& state, SourceSpan span)>;

	public:
		CallbackRule(const std::string& name, Callback&& callback, bool createNode = true);
		CallbackRule(std::string&& name, Callback&& callback, bool createNode = true);

		virtual void         cleanUp() override {}
		virtual MatchResult  match(MatcherState& state, SourceSpan span) override;

	private:
		Callback m_Callback;
	};

	template <Matcher Matcher>
	MatcherRule::MatcherRule(const std::string& name, Matcher&& matcher, bool createNode)
	    : IRule(name, createNode), m_Matcher(std::make_unique<Matcher>(std::move(matcher)))
	{
	}

	template <Matcher Matcher>
	MatcherRule::MatcherRule(std::string&& name, Matcher&& matcher, bool createNode)
	    : IRule(std::move(name), createNode), m_Matcher(std::make_unique<Matcher>(std::move(matcher)))
	{
	}
} // namespace CommonLexer