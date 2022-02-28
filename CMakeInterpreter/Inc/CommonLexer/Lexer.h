#pragma once

#include "Source.h"

#include <cstddef>

#include <functional>
#include <regex>
#include <set>
#include <string>
#include <vector>

namespace CommonLexer
{
	struct LexNode;
	struct Lex;
	class Lexer;

	enum class EMessageSeverity
	{
		Warning,
		Error
	};

	struct Message
	{
	public:
		Message();
		Message(const std::string& message, const SourceSpan& span, EMessageSeverity severity = EMessageSeverity::Error);
		Message(std::string&& message, SourceSpan&& span, EMessageSeverity severity = EMessageSeverity::Error);

		[[nodiscard]] auto& getMessage() const { return m_Message; }
		[[nodiscard]] auto& getSpan() const { return m_Span; }
		[[nodiscard]] auto  getSeverity() const { return m_Severity; }

	private:
		std::string      m_Message;
		SourceSpan       m_Span;
		EMessageSeverity m_Severity;
	};

	enum class ELexStatus
	{
		Success,
		Skip,
		Failure
	};

	struct LexResult
	{
	public:
		LexResult(ELexStatus status);
		LexResult(ELexStatus status, const SourceSpan& span);
		LexResult(ELexStatus status, SourceSpan&& span);
		LexResult(const LexResult& copy)     = default;
		LexResult(LexResult&& move) noexcept = default;

		LexResult& operator=(const LexResult& copy) = default;
		LexResult& operator=(LexResult&& move) noexcept = default;

		ELexStatus m_Status;
		SourceSpan m_Span;
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

		void setSource(ISource* source);
		void addMessage(const Message& message);
		void addMessage(Message&& message);

		[[nodiscard]] auto  getLexer() const { return m_Lexer; }
		[[nodiscard]] auto  getSource() const { return m_Source; }
		[[nodiscard]] auto& getRoot() { return m_Root; }
		[[nodiscard]] auto& getRoot() const { return m_Root; }
		[[nodiscard]] auto& getMessages() { return m_Messages; }
		[[nodiscard]] auto& getMessages() const { return m_Messages; }

	private:
		Lexer*               m_Lexer;
		ISource*             m_Source;
		LexNode              m_Root;
		std::vector<Message> m_Messages;
	};

	struct LexMatcher
	{
	public:
		virtual ~LexMatcher() = default;

		virtual LexResult lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) = 0;
	};

	struct LexMatcherRegex : public LexMatcher
	{
	public:
		explicit LexMatcherRegex(const std::string& regex);
		explicit LexMatcherRegex(std::string&& regex);

		virtual LexResult lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		std::regex m_Regex;
	};

	struct LexMatcherCombination : public LexMatcher
	{
	public:
		LexMatcherCombination(const std::vector<LexMatcher*>& matchers);
		LexMatcherCombination(std::vector<LexMatcher*>&& matchers);
		~LexMatcherCombination();

		virtual LexResult lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

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
		~LexMatcherMultiple();

		virtual LexResult lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		LexMatcher*             m_Matcher;
		ELexMatcherMultipleType m_Type;
	};

	struct LexMatcherOptional : public LexMatcher
	{
	public:
		LexMatcherOptional(LexMatcher* matcher);
		~LexMatcherOptional();

		virtual LexResult lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		LexMatcher* m_Matcher;
	};

	struct LexMatcherOr : public LexMatcher
	{
	public:
		LexMatcherOr(const std::vector<LexMatcher*>& matchers);
		LexMatcherOr(std::vector<LexMatcher*>&& matchers);
		~LexMatcherOr();

		virtual LexResult lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		std::vector<LexMatcher*> m_Matchers;
	};

	struct LexMatcherGroup : public LexMatcher
	{
	public:
		LexMatcherGroup(LexMatcher* matcher);
		~LexMatcherGroup();

		virtual LexResult lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		LexMatcher* m_Matcher;
	};

	struct LexMatcherBranch : public LexMatcher
	{
	public:
		LexMatcherBranch(const std::vector<LexMatcher*>& matchers);
		LexMatcherBranch(std::vector<LexMatcher*>&& matchers);
		~LexMatcherBranch();

		virtual LexResult lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		std::vector<LexMatcher*> m_Matchers;
	};

	struct LexMatcherNamedGroup : public LexMatcher
	{
	public:
		LexMatcherNamedGroup(const std::string& name, LexMatcher* matcher);
		LexMatcherNamedGroup(std::string&& name, LexMatcher* matcher);
		~LexMatcherNamedGroup();

		virtual LexResult lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		std::string m_Name;
		LexMatcher* m_Matcher;
	};

	struct LexMatcherNamedGroupReference : public LexMatcher
	{
	public:
		LexMatcherNamedGroupReference(const std::string& namedGroup);
		LexMatcherNamedGroupReference(std::string&& namedGroup);

		virtual LexResult lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		std::string m_NamedGroup;
	};

	struct LexMatcherReference : public LexMatcher
	{
	public:
		LexMatcherReference(const std::string& rule);
		LexMatcherReference(std::string&& rule);

		virtual LexResult lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		std::string m_Rule;
	};

	struct LexMatcherCallback : public LexMatcher
	{
	public:
		using Callback = std::function<LexResult(Lex&, LexNode&, ISource*, const SourceSpan&)>;

	public:
		LexMatcherCallback(const Callback& callback);
		LexMatcherCallback(Callback&& callback);

		virtual LexResult lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span) override;

	private:
		Callback m_Callback;
	};

	struct LexRule
	{
	public:
		LexRule(const std::string& name, LexMatcher* matcher, bool createNode = true);
		LexRule(std::string&& name, LexMatcher* matcher, bool createNode = true);
		LexRule(LexRule&& move) noexcept;
		~LexRule();

		[[nodiscard]] auto& getName() const { return m_Name; }

		LexResult lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span);

	private:
		std::string m_Name;

		LexMatcher* m_Matcher;
		bool        m_CreateNode;
	};

	class Lexer
	{
	public:
		Lex lexSource(ISource* source);
		Lex lexSource(ISource* source, const SourceSpan& span);

		void     registerRule(LexRule&& rule);
		void     setMainRule(const std::string& mainRule);
		LexRule* getRule(const std::string& rule);

		void       setGroupedValue(const std::string& group, const SourceSpan& span);
		void       setGroupedValue(std::string&& group, SourceSpan&& span);
		SourceSpan getGroupedValue(const std::string& group) const;

	private:
		std::vector<LexRule> m_Rules;
		std::string          m_MainRule;

		std::unordered_map<std::string, SourceSpan> m_GroupedValues;
	};
} // namespace CommonLexer