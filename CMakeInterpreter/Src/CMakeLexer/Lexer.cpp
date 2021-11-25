#include "CMakeLexer/Lexer.h"

namespace CMakeLexer {
	Lexer::Lexer()
	    : m_TFile("file"),
	      m_TFileElement("fileElement"),
	      m_TCommandInvocation("commandInvocation"),
	      m_TIdentifier("identifier"),
	      m_TArguments("arguments"),
	      m_TBracketContent("bracketContent"),
	      m_TQuotedArgument("quotedArgument"),
	      m_TQuotedElement("quotedElement"),
	      m_TUnquotedArgument("unquotedArgument"),
	      m_TUnquotedElement("unquotedElement"),
	      m_TUnquotedLegacy("unquotedLegacy"),
	      m_TEscapeSequence("escapeSequence"),
	      m_TLineComment("lineComment"),
	      m_TBracketComment("bracketComment") {
	}

	CommonLexer::Lex Lexer::lexSource(CommonLexer::ISource* source) {
		CommonLexer::Lex lex { source };
		lex.getRoot() = CommonLexer::LexNode(&m_TFile);
		return lex;
	}
} // namespace CMakeLexer
