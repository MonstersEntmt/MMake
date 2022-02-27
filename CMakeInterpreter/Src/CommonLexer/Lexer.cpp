#include "CommonLexer/Lexer.h"

#include <utility>

namespace CommonLexer
{
	Message::Message()
	    : m_Message({}), m_Span({}), m_Severity(EMessageSeverity::Warning) {}

	Message::Message(const std::string& message, const SourceSpan& span, EMessageSeverity severity)
	    : m_Message(message), m_Span(span), m_Severity(severity) {}

	Message::Message(std::string&& message, SourceSpan&& span, EMessageSeverity severity)
	    : m_Message(std::move(message)), m_Span(std::move(span)), m_Severity(severity) {}

	LexResult::LexResult(ELexStatus status)
	    : m_Status(status), m_Span({}) {}

	LexResult::LexResult(ELexStatus status, const SourceSpan& span)
	    : m_Status(status), m_Span(span) {}

	LexResult::LexResult(ELexStatus status, SourceSpan&& span)
	    : m_Status(status), m_Span(std::move(span)) {}

	LexNode::LexNode(Lexer& lexer)
	    : m_Lexer(&lexer), m_Rule({}), m_Source(nullptr), m_Span({}) {}

	LexNode::LexNode(Lexer& lexer, const std::string& rule)
	    : m_Lexer(&lexer), m_Rule(rule), m_Source(nullptr), m_Span({}) {}

	LexNode::LexNode(Lexer& lexer, std::string&& rule)
	    : m_Lexer(&lexer), m_Rule(std::move(rule)), m_Source(nullptr), m_Span({}) {}

	LexNode* LexNode::getChild(std::size_t index) const
	{
		return index < m_Children.size() ? const_cast<LexNode*>(&m_Children[index]) : nullptr;
	}

	void LexNode::setLexer(Lexer& lexer)
	{
		m_Lexer = &lexer;
	}

	void LexNode::setRule(const std::string& rule)
	{
		m_Rule = rule;
	}

	void LexNode::setRule(std::string&& rule)
	{
		m_Rule = std::move(rule);
	}

	void LexNode::setSourceSpan(ISource* source, const SourceSpan& span)
	{
		m_Source = source;
		m_Span   = span;
	}

	void LexNode::addChild(const LexNode& child)
	{
		m_Children.push_back(child);
	}

	void LexNode::addChild(LexNode&& child)
	{
		m_Children.push_back(std::move(child));
	}

	Lex::Lex(Lexer& lexer)
	    : m_Lexer(&lexer), m_Source(nullptr), m_Root(lexer) {}

	Lex::Lex(Lexer& lexer, ISource* source)
	    : m_Lexer(&lexer), m_Source(source), m_Root(lexer) {}

	void Lex::setSource(ISource* source)
	{
		m_Source = source;
	}

	void Lex::addMessage(const Message& message)
	{
		m_Messages.push_back(message);
	}

	void Lex::addMessage(Message&& message)
	{
		m_Messages.push_back(std::move(message));
	}

	LexMatcherRegex::LexMatcherRegex(const std::string& regex)
	    : m_Regex("^" + regex, std::regex_constants::ECMAScript | std::regex_constants::optimize) {}

	LexMatcherRegex::LexMatcherRegex(std::string&& regex)
	    : m_Regex("^" + std::move(regex), std::regex_constants::ECMAScript | std::regex_constants::optimize) {}

	LexResult LexMatcherRegex::lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span)
	{
		std::match_results<SourceIterator> results;
		if (std::regex_search(span.begin(source), span.end(source), results, m_Regex))
			return { ELexStatus::Success, { results[0].first, results[0].second } };
		return ELexStatus::Failure;
	}

	LexMatcherCombination::LexMatcherCombination(const std::vector<LexMatcher*>& matchers)
	    : m_Matchers(matchers) {}

	LexMatcherCombination::LexMatcherCombination(std::vector<LexMatcher*>&& matchers)
	    : m_Matchers(std::move(matchers)) {}

	LexMatcherCombination::~LexMatcherCombination()
	{
		for (auto matcher : m_Matchers)
			delete matcher;
		m_Matchers.clear();
	}

	LexResult LexMatcherCombination::lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span)
	{
		SourceSpan totalSpan = { span.m_Begin, span.m_Begin };
		SourceSpan subSpan   = span;
		for (auto matcher : m_Matchers)
		{
			auto result = matcher->lex(lex, parentNode, source, subSpan);
			switch (result.m_Status)
			{
			case ELexStatus::Failure:
				return ELexStatus::Failure;
			case ELexStatus::Skip:
				// TODO(MarcasRealAccount): WTF should I do here???
				return ELexStatus::Skip;
			case ELexStatus::Success:
				subSpan.m_Begin = result.m_Span.m_End;
				totalSpan.m_End = result.m_Span.m_End;
				break;
			}
		}
		return { ELexStatus::Success, totalSpan };
	}

	LexMatcherMultiple::LexMatcherMultiple(LexMatcher* matcher, ELexMatcherMultipleType type)
	    : m_Matcher(matcher), m_Type(type) {}

	LexMatcherMultiple::~LexMatcherMultiple()
	{
		delete m_Matcher;
	}

	LexResult LexMatcherMultiple::lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span)
	{
		std::size_t matches   = 0;
		SourceSpan  totalSpan = { span.m_Begin, span.m_Begin };
		SourceSpan  subSpan   = span;

		while (true)
		{
			auto result = m_Matcher->lex(lex, parentNode, source, subSpan);
			if (result.m_Status != ELexStatus::Success)
				break;

			subSpan.m_Begin = result.m_Span.m_End;
			totalSpan.m_End = result.m_Span.m_End;
		}

		switch (m_Type)
		{
		case ELexMatcherMultipleType::OneOrMore:
			return matches > 0 ? LexResult { ELexStatus::Success, totalSpan } : ELexStatus::Failure;
		case ELexMatcherMultipleType::ZeroOrMore:
			return { ELexStatus::Success, totalSpan };
		}
	}

	LexMatcherOptional::LexMatcherOptional(LexMatcher* matcher)
	    : m_Matcher(matcher) {}

	LexMatcherOptional::~LexMatcherOptional()
	{
		delete m_Matcher;
	}

	LexResult LexMatcherOptional::lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span)
	{
		auto result = m_Matcher->lex(lex, parentNode, source, span);
		switch (result.m_Status)
		{
		case ELexStatus::Success:
			return result;
		case ELexStatus::Skip: [[fallthrough]];
		case ELexStatus::Failure:
			return ELexStatus::Skip;
			return ELexStatus::Skip;
		}
	}

	LexMatcherOr::LexMatcherOr(const std::vector<LexMatcher*>& matchers)
	    : m_Matchers(matchers) {}

	LexMatcherOr::LexMatcherOr(std::vector<LexMatcher*>&& matchers)
	    : m_Matchers(std::move(matchers)) {}

	LexMatcherOr::~LexMatcherOr()
	{
		for (auto matcher : m_Matchers)
			delete matcher;
		m_Matchers.clear();
	}

	LexResult LexMatcherOr::lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span)
	{
		ELexStatus status       = ELexStatus::Failure;
		SourceSpan smallestSpan = span;
		for (auto matcher : m_Matchers)
		{
			auto result = matcher->lex(lex, parentNode, source, span);
			if (status == ELexStatus::Failure && result.m_Status == ELexStatus::Skip)
				status = ELexStatus::Skip;
			if (result.m_Status != ELexStatus::Success)
				continue;
			status = ELexStatus::Success;
			if (result.m_Span.length() < smallestSpan.length())
				smallestSpan = result.m_Span;
		}

		switch (status)
		{
		case ELexStatus::Failure: return ELexStatus::Failure;
		case ELexStatus::Skip: return ELexStatus::Skip;
		case ELexStatus::Success: return { ELexStatus::Success, smallestSpan };
		}
	}

	LexMatcherGroup::LexMatcherGroup(LexMatcher* matcher)
	    : m_Matcher(matcher) {}

	LexMatcherGroup::~LexMatcherGroup()
	{
		delete m_Matcher;
	}

	LexResult LexMatcherGroup::lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span)
	{
		return m_Matcher->lex(lex, parentNode, source, span);
	}

	LexMatcherBranch::LexMatcherBranch(const std::vector<LexMatcher*>& matchers)
	    : m_Matchers(matchers) {}

	LexMatcherBranch::LexMatcherBranch(std::vector<LexMatcher*>&& matchers)
	    : m_Matchers(std::move(matchers)) {}

	LexMatcherBranch::~LexMatcherBranch()
	{
		for (auto matcher : m_Matchers)
			delete matcher;
		m_Matchers.clear();
	}

	LexResult LexMatcherBranch::lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span)
	{
		ELexStatus status       = ELexStatus::Failure;
		SourceSpan smallestSpan = span;
		for (auto matcher : m_Matchers)
		{
			auto result = matcher->lex(lex, parentNode, source, span);
			if (status == ELexStatus::Failure && result.m_Status == ELexStatus::Skip)
				status = ELexStatus::Skip;
			if (result.m_Status != ELexStatus::Success)
				continue;
			status = ELexStatus::Success;
			if (result.m_Span.length() < smallestSpan.length())
				smallestSpan = result.m_Span;
		}

		switch (status)
		{
		case ELexStatus::Failure: return ELexStatus::Failure;
		case ELexStatus::Skip: return ELexStatus::Skip;
		case ELexStatus::Success: return { ELexStatus::Success, smallestSpan };
		}
	}

	LexMatcherNamedGroup::LexMatcherNamedGroup(const std::string& name, LexMatcher* matcher)
	    : m_Name(name), m_Matcher(matcher) {}

	LexMatcherNamedGroup::LexMatcherNamedGroup(std::string&& name, LexMatcher* matcher)
	    : m_Name(std::move(name)), m_Matcher(matcher) {}

	LexMatcherNamedGroup::~LexMatcherNamedGroup()
	{
		delete m_Matcher;
	}

	LexResult LexMatcherNamedGroup::lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span)
	{
		auto result = m_Matcher->lex(lex, parentNode, source, span);
		switch (result.m_Status)
		{
		case ELexStatus::Failure: return ELexStatus::Failure;
		case ELexStatus::Skip: return ELexStatus::Skip;
		case ELexStatus::Success:
			lex.getLexer()->setGroupedValue(m_Name, result.m_Span);
			return { ELexStatus::Success, result.m_Span };
		}
	}

	LexMatcherNamedGroupReference::LexMatcherNamedGroupReference(const std::string& namedGroup)
	    : m_NamedGroup(namedGroup) {}

	LexMatcherNamedGroupReference::LexMatcherNamedGroupReference(std::string&& namedGroup)
	    : m_NamedGroup(std::move(namedGroup)) {}

	LexResult LexMatcherNamedGroupReference::lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span)
	{
		auto value = lex.getLexer()->getGroupedValue(m_NamedGroup);
		if (value.length() > span.length())
			return ELexStatus::Failure;

		auto sourceItr = span.begin(source);
		auto sourceEnd = span.end(source);
		auto valueItr  = value.begin(source);
		auto valueEnd  = value.end(source);
		while (sourceItr != sourceEnd && valueItr != valueEnd)
		{
			if (*sourceItr != *valueItr)
				return ELexStatus::Failure;

			++sourceItr;
			++valueItr;
		}
		return { ELexStatus::Success, { span.m_Begin, sourceItr } };
	}

	LexMatcherReference::LexMatcherReference(const std::string& rule)
	    : m_Rule(rule) {}

	LexMatcherReference::LexMatcherReference(std::string&& rule)
	    : m_Rule(std::move(rule)) {}

	LexResult LexMatcherReference::lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span)
	{
		auto rule = lex.getLexer()->getRule(m_Rule);
		if (!rule)
			return ELexStatus::Failure;

		return rule->lex(lex, parentNode, source, span);
	}

	LexMatcherCallback::LexMatcherCallback(const Callback& callback)
	    : m_Callback(callback) {}

	LexMatcherCallback::LexMatcherCallback(Callback&& callback)
	    : m_Callback(std::move(callback)) {}

	LexResult LexMatcherCallback::lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span)
	{
		return m_Callback(lex, parentNode, source, span);
	}

	LexRule::LexRule(const std::string& name, LexMatcher* matcher, bool createNode)
	    : m_Name(name), m_Matcher(matcher), m_CreateNode(createNode) {}

	LexRule::LexRule(std::string&& name, LexMatcher* matcher, bool createNode)
	    : m_Name(std::move(name)), m_Matcher(matcher), m_CreateNode(createNode) {}

	LexRule::LexRule(LexRule&& move)
	    : m_Name(std::move(move.m_Name)), m_Matcher(move.m_Matcher), m_CreateNode(move.m_CreateNode)
	{
		move.m_Matcher = nullptr;
	}

	LexRule::~LexRule()
	{
		delete m_Matcher;
	}

	LexResult LexRule::lex(Lex& lex, LexNode& parentNode, ISource* source, const SourceSpan& span)
	{
		auto result = m_Matcher->lex(lex, parentNode, source, span);
		switch (result.m_Status)
		{
		case ELexStatus::Failure: return ELexStatus::Failure;
		case ELexStatus::Skip: return ELexStatus::Skip;
		case ELexStatus::Success:
			if (m_CreateNode)
			{
				LexNode currentNode { *lex.getLexer(), m_Name };
				currentNode.setSourceSpan(source, result.m_Span);
				parentNode.addChild(std::move(currentNode));
			}
			return { ELexStatus::Success, result.m_Span };
		}
	}

	Lex Lexer::lexSource(ISource* source)
	{
		return lexSource(source, source->getCompleteSpan());
	}

	Lex Lexer::lexSource(ISource* source, const SourceSpan& span)
	{
		m_GroupedValues.clear();

		Lex  lex { *this, source };
		auto rule = getRule(m_MainRule);
		if (rule && source)
			rule->lex(lex, lex.getRoot(), source, span);
		return lex;
	}

	void Lexer::registerRule(LexRule&& rule)
	{
		m_Rules.push_back(std::move(rule));
	}

	void Lexer::setMainRule(const std::string& mainRule)
	{
		m_MainRule = mainRule;
	}

	LexRule* Lexer::getRule(const std::string& rule)
	{
		for (auto& rl : m_Rules)
			if (rl.getName() == rule)
				return &rl;
		return nullptr;
	}

	void Lexer::setGroupedValue(const std::string& group, const SourceSpan& span)
	{
		m_GroupedValues.insert_or_assign(group, span);
	}

	void Lexer::setGroupedValue(std::string&& group, SourceSpan&& span)
	{
		m_GroupedValues.insert_or_assign(std::move(group), std::move(span));
	}

	SourceSpan Lexer::getGroupedValue(const std::string& group) const
	{
		auto itr = m_GroupedValues.find(group);
		return itr != m_GroupedValues.end() ? itr->second : SourceSpan {};
	}
} // namespace CommonLexer