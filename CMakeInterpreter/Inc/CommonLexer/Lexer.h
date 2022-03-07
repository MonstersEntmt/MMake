#pragma once

#include "Message.h"
#include "Node.h"
#include "Rule.h"
#include "Source.h"

#include <unordered_map>

namespace CommonLexer
{
	struct Lex
	{
	public:
		explicit Lex(Lexer& lexer);
		Lex(Lexer& lexer, ISource* source);

		void setSource(ISource* source);
		void setMessages(std::vector<Message>&& messages);

		[[nodiscard]] auto  getLexer() const { return m_Lexer; }
		[[nodiscard]] auto  getSource() const { return m_Source; }
		[[nodiscard]] auto& getRoot() { return m_Root; }
		[[nodiscard]] auto& getRoot() const { return m_Root; }
		[[nodiscard]] auto& getMessages() { return m_Messages; }
		[[nodiscard]] auto& getMessages() const { return m_Messages; }

	private:
		Lexer*               m_Lexer;
		ISource*             m_Source;
		Node                 m_Root;
		std::vector<Message> m_Messages;
	};

	class Lexer
	{
	public:
		Lex lexSource(ISource* source);
		Lex lexSource(ISource* source, SourceSpan span);

		template <Rule Rule>
		void   registerRule(Rule&& rule);
		void   registerRule(std::unique_ptr<IRule>&& rule);
		IRule* getRule(const std::string& rule);
		void   setMainRule(const std::string& mainRule);

		void       setGroupedValue(const std::string& group, SourceSpan span);
		void       setGroupedValue(std::string&& group, SourceSpan span);
		SourceSpan getGroupedValue(const std::string& group) const;

	private:
		std::string m_MainRule;

		std::vector<std::unique_ptr<IRule>> m_Rules;

		std::unordered_map<std::string, SourceSpan> m_GroupedValues;
	};

	template <Rule Rule>
	void Lexer::registerRule(Rule&& rule)
	{
		m_Rules.push_back(std::make_unique<Rule>(std::move(rule)));
	}
} // namespace CommonLexer