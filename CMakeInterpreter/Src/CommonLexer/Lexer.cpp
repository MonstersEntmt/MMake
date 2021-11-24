#include "CommonLexer/Lexer.h"

#include <utility>

namespace CommonLexer {
	LexNodeType::LexNodeType(const std::string& name)
	    : m_Name(name) { }

	LexNodeType::LexNodeType(std::string&& name)
	    : m_Name(std::move(name)) { }

	LexNode::LexNode()
	    : m_Type(nullptr), m_Source(nullptr) { }

	LexNode::LexNode(LexNodeType* type)
	    : m_Type(type), m_Source(nullptr) { }

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

	Lex::Lex(ISource* source)
	    : m_Source(source) { }

	void Lexer::registerNodeType(LexNodeType* nodeType) {
		m_NodeTypes.insert(nodeType);
	}
} // namespace CommonLexer