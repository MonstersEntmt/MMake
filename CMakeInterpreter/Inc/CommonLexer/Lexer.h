#pragma once

#include "Source.h"

#include <cstddef>

#include <set>
#include <string>
#include <vector>

namespace CommonLexer {
	class LexNodeType {
	public:
		LexNodeType(const std::string& name);
		LexNodeType(std::string&& name);

		auto& getName() const { return m_Name; }

	private:
		std::string m_Name;
	};

	struct LexNode {
	public:
		LexNode();
		LexNode(LexNodeType* type);

		void setType(LexNodeType* type);
		void setSourceSpan(ISource* source, const SourceSpan& span);
		void addChild(const LexNode& child);
		void addChild(LexNode&& child);

		auto getType() const { return m_Type; }
		auto getSource() const { return m_Source; }
		auto& getSpan() const { return m_Span; }
		auto& getChildren() const { return m_Children; }

	private:
		LexNodeType* m_Type;
		ISource* m_Source;
		SourceSpan m_Span;
		std::vector<LexNode> m_Children;
	};

	struct Lex {
	public:
		Lex(ISource* source);

		auto getSource() const { return m_Source; }
		auto& getRoot() { return m_Root; }
		auto& getRoot() const { return m_Root; }

	private:
		ISource* m_Source;
		LexNode m_Root;
	};

	class Lexer {
	public:
		virtual Lex lexSource(ISource* source) = 0;

		void registerNodeType(LexNodeType* nodeType);

		auto& getNodeTypes() { return m_NodeTypes; }
		auto& getNodeTypes() const { return m_NodeTypes; }

	private:
		std::set<LexNodeType*> m_NodeTypes;
	};
} // namespace CommonLexer