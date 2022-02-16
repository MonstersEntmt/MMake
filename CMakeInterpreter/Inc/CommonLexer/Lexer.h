#pragma once

#include "Source.h"

#include <cstddef>

#include <functional>
#include <set>
#include <string>
#include <vector>

namespace CommonLexer
{
	struct LexNode;
	struct Lex;
	struct Lexer;

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

	enum class ELexResult
	{
		Success,
		Skip,
		Failure
	};

	struct LexNode
	{
	public:
		explicit LexNode(Lexer& lexer);
		LexNode(Lexer& lexer, const std::string& rule);
		LexNode(Lexer& lexer, std::string&& rule);

		[[nodiscard]] LexNode* getChild(std::size_t index) const;

		void setLexer(Lexer& lexer);
		void setRule(const std::string& rule);
		void setRule(std::string&& rule);
		void setSourceSpan(ISource* source, const SourceSpan& span);
		void addChild(const LexNode& child);
		void addChild(LexNode&& child);

		[[nodiscard]] auto  getLexer() const { return m_Lexer; }
		[[nodiscard]] auto& getRule() const { return m_Rule; }
		[[nodiscard]] auto  getSource() const { return m_Source; }
		[[nodiscard]] auto& getSpan() const { return m_Span; }
		[[nodiscard]] auto& getChildren() const { return m_Children; }

	private:
		Lexer*               m_Lexer;
		std::string          m_Rule;
		ISource*             m_Source;
		SourceSpan           m_Span;
		std::vector<LexNode> m_Children;
	};

	struct Lex
	{
	public:
		explicit Lex(Lexer& lexer);
		Lex(Lexer& lexer, ISource* source);

		[[nodiscard]] auto  getLexer() const { return m_Lexer; }
		[[nodiscard]] auto  getSource() const { return m_Source; }
		[[nodiscard]] auto& getRoot() { return m_Root; }
		[[nodiscard]] auto& getRoot() const { return m_Root; }
		[[nodiscard]] auto& getMessages() { return m_Messages; }
		[[nodiscard]] auto& getMessages() const { return m_Messages; }

	private:
		Lexer*                    m_Lexer;
		ISource*                  m_Source;
		LexNode                   m_Root;
		std::vector<LexerMessage> m_Messages;
	};

	struct LexMatcher
	{
	public:
		virtual ELexResult lex(Lexer& lexer, Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) = 0;
	};

	struct LexMatcherRegex : public LexMatcher
	{
	public:
		LexMatcherRegex(const std::string& regex);
		LexMatcherRegex(std::string&& regex);

		virtual ELexResult lex(Lexer& lexer, Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		std::string m_Regex;
	};

	struct LexMatcherCombination : public LexMatcher
	{
	public:
		LexMatcherCombination(const std::vector<LexMatcher*>& matchers);
		LexMatcherCombination(std::vector<LexMatcher*>&& matchers);

		virtual ELexResult lex(Lexer& lexer, Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		std::vector<LexMatcher*> m_Matchers;
	};

	enum class ELexMatcherMultipleType
	{
		ZeroOrMore,
		OneOrMore
	};

	struct LexMatcherMultiple : public LexMatcher
	{
	public:
		LexMatcherMultiple(LexMatcher* matcher, ELexMatcherMultipleType type = ELexMatcherMultipleType::ZeroOrMore);

		virtual ELexResult lex(Lexer& lexer, Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		LexMatcher*             m_Matcher;
		ELexMatcherMultipleType m_Type;
	};

	struct LexMatcherOptional : public LexMatcher
	{
	public:
		LexMatcherOptional(LexMatcher* matcher);

		virtual ELexResult lex(Lexer& lexer, Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		LexMatcher* m_Matcher;
	};

	struct LexMatcherOr : public LexMatcher
	{
	public:
		LexMatcherOr(const std::vector<LexMatcher*>& matchers);
		LexMatcherOr(std::vector<LexMatcher*>&& matchers);

		virtual ELexResult lex(Lexer& lexer, Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		std::vector<LexMatcher*> m_Matchers;
	};

	struct LexMatcherGroup : public LexMatcher
	{
	public:
		LexMatcherGroup(LexMatcher* matcher);

		virtual ELexResult lex(Lexer& lexer, Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		LexMatcher* m_Matcher;
	};

	struct LexMatcherBranch : public LexMatcher
	{
	public:
		LexMatcherBranch(const std::vector<LexMatcher*>& matchers);
		LexMatcherBranch(std::vector<LexMatcher*>&& matchers);

		virtual ELexResult lex(Lexer& lexer, Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		std::vector<LexMatcher*> m_Matchers;
	};

	struct LexMatcherNamedGroup : public LexMatcher
	{
	public:
		LexMatcherNamedGroup(const std::string& name, LexMatcher* matcher);
		LexMatcherNamedGroup(std::string&& name, LexMatcher* matcher);

		virtual ELexResult lex(Lexer& lexer, Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		std::string m_Name;
		LexMatcher* m_Matcher;
	};

	struct LexMatcherNamedGroupReference : public LexMatcher
	{
	public:
		LexMatcherNamedGroupReference(const std::string& namedGroup);
		LexMatcherNamedGroupReference(std::string&& namedGroup);

		virtual ELexResult lex(Lexer& lexer, Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		std::string m_NamedGroup;
	};

	struct LexMatcherReference : public LexMatcher
	{
	public:
		LexMatcherReference(const std::string& rule);
		LexMatcherReference(std::string&& rule);

		virtual ELexResult lex(Lexer& lexer, Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		std::string m_Rule;
	};

	struct LexMatcherCallback : public LexMatcher
	{
	public:
		using Callback = std::function<ELexResult(Lexer&, Lex&, LexNode&, ISource*, const SourceSpan&)>;

	public:
		LexMatcherCallback(Callback callback);

		virtual ELexResult lex(Lexer& lexer, Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		Callback m_Callback;
	};

	struct LexRule
	{
	public:
		LexRule(const std::string& name, LexMatcher* matcher, bool createNode = true);
		LexRule(std::string&& name, LexMatcher* matcher, bool createNode = true);
		LexRule(LexRule&& move);
		~LexRule();

		auto& getName() const { return m_Name; }

		ELexResult lex(Lexer& lexer, Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span);

	private:
		std::string m_Name;

		LexMatcher* m_Matcher;
		bool        m_CreateNode;
	};

	class Lexer
	{
	public:
		Lex lexSource(ISource* source);

		void registerRule(LexRule&& rule);
		void setMainRule(const std::string& mainRule);

	private:
		std::vector<LexRule> m_Rules;
		std::string          m_MainRule;
	};
} // namespace CommonLexer