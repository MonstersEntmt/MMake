#include "CommonLexer/Lexer.h"

#include <utility>

namespace CommonLexer {
	LexerMessage::LexerMessage()
	: m_Severity(LexerMessageSeverity::Error) {}
	
	LexerMessage::LexerMessage(const std::string& message, const SourceSpan& span, LexerMessageSeverity severity)
		: m_Message(message), m_Span(span), m_Severity(severity) { }
	
	LexerMessage::LexerMessage(std::string&& message, SourceSpan&& span, LexerMessageSeverity severity)
		: m_Message(std::move(message)), m_Span(std::move(span)), m_Severity(severity) { }
	
	LexNodeType::LexNodeType(const std::string& name)
	    : m_Name(name) { }

	LexNodeType::LexNodeType(std::string&& name)
	    : m_Name(std::move(name)) { }

	LexNode::LexNode()
	    : m_Type(nullptr), m_Source(nullptr) { }

	LexNode::LexNode(LexNodeType* type)
	    : m_Type(type), m_Source(nullptr) { }
	
	LexNode* LexNode::getChild(std::size_t index) const {
		return index < m_Children.size() ? const_cast<LexNode*>(&m_Children[index]) : nullptr;
	}

	void LexNode::setType(LexNodeType* type) {
		m_Type = type;
	}

	void LexNode::setSourceSpan(ISource* source, const SourceSpan& span) {
		m_Source = source;
		m_Span   = span;
	}

	void LexNode::addChild(const LexNode& child) {
		m_Children.push_back(child);
	}

	void LexNode::addChild(LexNode&& child) {
		m_Children.push_back(std::move(child));
	}
	
	Lex::Lex()
		: m_Source(nullptr) { }

	Lex::Lex(ISource* source)
	    : m_Source(source) { }

	void Lexer::registerNodeType(LexNodeType* nodeType) {
		m_NodeTypes.insert(nodeType);
	}
} // namespace CommonLexer
