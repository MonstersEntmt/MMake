#include "CommonLexer/Node.h"

namespace CommonLexer
{
	Node::Node(Lexer& lexer)
	    : m_Lexer(&lexer), m_Source(nullptr) {}

	Node::Node(Lexer& lexer, const std::string& rule)
	    : m_Lexer(&lexer), m_Rule(rule), m_Source(nullptr) {}

	Node::Node(Lexer& lexer, std::string&& rule)
	    : m_Lexer(&lexer), m_Rule(std::move(rule)), m_Source(nullptr) {}

	void Node::setLexer(Lexer& lexer)
	{
		m_Lexer = &lexer;
	}

	void Node::setRule(const std::string& rule)
	{
		m_Rule = rule;
	}

	void Node::setRule(std::string&& rule)
	{
		m_Rule = std::move(rule);
	}

	void Node::setSourceSpan(ISource* source, SourceSpan span)
	{
		m_Source = source;
		m_Span   = span;
	}

	void Node::addChild(const Node& child)
	{
		m_Children.push_back(child);
	}

	void Node::addChild(Node&& child)
	{
		m_Children.push_back(std::move(child));
	}

	void Node::addChildren(const Node& node)
	{
		m_Children.reserve(m_Children.size() + node.m_Children.size());
		for (auto& child : node.m_Children)
			m_Children.push_back(child);
	}

	void Node::addChildren(Node&& node)
	{
		m_Children.reserve(m_Children.size() + node.m_Children.size());
		for (auto& child : node.m_Children)
			m_Children.push_back(std::move(child));
		node.m_Children.clear();
	}

	Node* Node::getChild(std::size_t index)
	{
		return index < m_Children.size() ? &m_Children[index] : nullptr;
	}

	const Node* Node::getChild(std::size_t index) const
	{
		return index < m_Children.size() ? &m_Children[index] : nullptr;
	}
} // namespace CommonLexer