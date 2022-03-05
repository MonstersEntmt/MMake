#pragma once

#include "Source.h"

#include <string>

namespace CommonLexer
{
	class Lexer;

	struct Node
	{
	public:
		explicit Node(Lexer& lexer);
		Node(Lexer& lexer, const std::string& rule);
		Node(Lexer& lexer, std::string&& rule);

		void setLexer(Lexer& lexer);
		void setRule(const std::string& rule);
		void setRule(std::string&& rule);
		void setSourceSpan(ISource* source, SourceSpan span);
		void addChild(const Node& child);
		void addChild(Node&& child);
		void addChildren(const Node& node);
		void addChildren(Node&& node);

		Node*               getChild(std::size_t index);
		const Node*         getChild(std::size_t index) const;
		[[nodiscard]] auto  getLexer() const { return m_Lexer; }
		[[nodiscard]] auto& getRule() const { return m_Rule; }
		[[nodiscard]] auto  getSource() const { return m_Source; }
		[[nodiscard]] auto  getSpan() const { return m_Span; }
		[[nodiscard]] auto& getChildren() const { return m_Children; }

	private:
		Lexer*            m_Lexer;
		std::string       m_Rule;
		ISource*          m_Source;
		SourceSpan        m_Span;
		std::vector<Node> m_Children;
	};
} // namespace CommonLexer