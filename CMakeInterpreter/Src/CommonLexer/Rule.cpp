#include "CommonLexer/Rule.h"

namespace CommonLexer
{
	IRule::IRule(const std::string& name, bool createNode)
	    : m_Name(name), m_CreateNode(createNode) {}

	IRule::IRule(std::string&& name, bool createNode)
	    : m_Name(std::move(name)), m_CreateNode(createNode) {}

	MatcherRule::MatcherRule(const std::string& name, std::unique_ptr<IMatcher>&& matcher, bool createNode)
	    : IRule(name, createNode), m_Matcher(std::move(matcher)) {}

	MatcherRule::MatcherRule(std::string&& name, std::unique_ptr<IMatcher>&& matcher, bool createNode)
	    : IRule(std::move(name), createNode), m_Matcher(std::move(matcher)) {}

	void MatcherRule::cleanUp()
	{
		m_Matcher->cleanUp();
	}

	MatchResult MatcherRule::match(MatcherState& state, SourceSpan span)
	{
		if (m_CreateNode)
		{
			Node         currentNode { *state.m_Lexer, m_Name };
			MatcherState tempState { {}, &currentNode, state.m_Lexer, state.m_Source, state.m_SourceSpan, this, span.m_Begin };
			auto         result = m_Matcher->match(tempState, span);
			if (result.m_Status == EMatchStatus::Success)
			{
				currentNode.setSourceSpan(state.m_Source, result.m_Span);
				state.m_ParentNode->addChild(std::move(currentNode));
			}
			state.m_Messages.reserve(state.m_Messages.size() + tempState.m_Messages.size());
			for (auto& message : tempState.m_Messages)
				state.m_Messages.push_back(std::move(message));
			return result;
		}
		else
		{
			MatcherState tempState { {}, state.m_ParentNode, state.m_Lexer, state.m_Source, state.m_SourceSpan, this, span.m_Begin };
			auto         result = m_Matcher->match(tempState, span);
			state.m_Messages.reserve(state.m_Messages.size() + tempState.m_Messages.size());
			for (auto& message : tempState.m_Messages)
				state.m_Messages.push_back(std::move(message));
			return result;
		}
	}

	CallbackRule::CallbackRule(const std::string& name, Callback&& callback, bool createNode)
	    : IRule(name, createNode), m_Callback(std::move(callback)) {}

	CallbackRule::CallbackRule(std::string&& name, Callback&& callback, bool createNode)
	    : IRule(std::move(name), createNode), m_Callback(std::move(callback)) {}

	MatchResult CallbackRule::match(MatcherState& state, SourceSpan span)
	{
		MatcherState tempState { {}, state.m_ParentNode, state.m_Lexer, state.m_Source, state.m_SourceSpan, this, span.m_Begin };
		auto         result = m_Callback(tempState, span);
		state.m_Messages.reserve(state.m_Messages.size() + tempState.m_Messages.size());
		for (auto& message : tempState.m_Messages)
			state.m_Messages.push_back(std::move(message));
		return result;
	}
} // namespace CommonLexer