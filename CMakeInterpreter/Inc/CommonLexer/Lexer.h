#pragma once

#include "Source.h"

#include <cstddef>

#include <set>
#include <string>
#include <vector>

namespace CommonLexer
{
	enum class LexerMessageSeverity
	{
		Warning,
		Error
	};

	struct LexerMessage
	{
	public:
		LexerMessage();
		LexerMessage(const std::string& message, const SourceSpan& span, LexerMessageSeverity severity = LexerMessageSeverity::Error);
		LexerMessage(std::string&& message, SourceSpan&& span, LexerMessageSeverity severity = LexerMessageSeverity::Error);

		[[nodiscard]] auto& getMessage() const { return m_Message; }
		[[nodiscard]] auto& getSpan() const { return m_Span; }
		[[nodiscard]] auto  getSeverity() const { return m_Severity; }

	private:
		std::string          m_Message;
		SourceSpan           m_Span;
		LexerMessageSeverity m_Severity;
	};

	class LexNodeType
	{
	public:
		explicit LexNodeType(const std::string& name);
		explicit LexNodeType(std::string&& name);

		[[nodiscard]] auto& getName() const { return m_Name; }

	private:
		std::string m_Name;
	};

	struct LexNode
	{
	public:
		LexNode();
		explicit LexNode(LexNodeType* type);

		[[nodiscard]] LexNode* getChild(std::size_t index) const;

		void setType(LexNodeType* type);
		void setSourceSpan(ISource* source, const SourceSpan& span);
		void addChild(const LexNode& child);
		void addChild(LexNode&& child);

		[[nodiscard]] auto  getType() const { return m_Type; }
		[[nodiscard]] auto  getSource() const { return m_Source; }
		[[nodiscard]] auto& getSpan() const { return m_Span; }
		[[nodiscard]] auto& getChildren() const { return m_Children; }

	private:
		LexNodeType*         m_Type;
		ISource*             m_Source;
		SourceSpan           m_Span;
		std::vector<LexNode> m_Children;
	};

	struct Lex
	{
	public:
		Lex();
		explicit Lex(ISource* source);

		[[nodiscard]] auto  getSource() const { return m_Source; }
		[[nodiscard]] auto& getRoot() { return m_Root; }
		[[nodiscard]] auto& getRoot() const { return m_Root; }
		[[nodiscard]] auto& getMessages() { return m_Messages; }
		[[nodiscard]] auto& getMessages() const { return m_Messages; }

	private:
		ISource*                  m_Source;
		LexNode                   m_Root;
		std::vector<LexerMessage> m_Messages;
	};

	class Lexer
	{
	public:
		virtual Lex lexSource(ISource* source) = 0;

		void registerNodeType(LexNodeType* nodeType);

		[[nodiscard]] auto& getNodeTypes() { return m_NodeTypes; }
		[[nodiscard]] auto& getNodeTypes() const { return m_NodeTypes; }

	private:
		std::set<LexNodeType*> m_NodeTypes;
	};
} // namespace CommonLexer