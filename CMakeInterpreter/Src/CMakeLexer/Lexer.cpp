#include "CMakeLexer/Lexer.h"

namespace CMakeLexer
{
	Lexer::Lexer()
	{
		using namespace CommonLexer;
		registerRule({ "file", new LexMatcherMultiple(new LexMatcherReference("fileElement")) });
		registerRule({ "fileElement",
		               new LexMatcherBranch(
		                   { new LexMatcherCombination(
		                         { new LexMatcherReference("commandInvocation"),
		                           new LexMatcherReference("lineEnding") }),
		                     new LexMatcherCombination(
		                         { new LexMatcherMultiple(new LexMatcherOr(
		                               { new LexMatcherReference("bracketComment"),
		                                 new LexMatcherReference("space") })),
		                           new LexMatcherReference("lineEnding") }) }) });
		registerRule({ "lineEnding",
		               new LexMatcherCombination(
		                   { new LexMatcherOptional(new LexMatcherReference("lineComment")),
		                     new LexMatcherReference("newline") }) });
		registerRule({ "space", new LexMatcherRegex("[ \t]+"), false });
		registerRule({ "newline", new LexMatcherRegex("\n"), false });

		registerRule({ "commandInvocation", new LexMatcherCombination(
		                                        { new LexMatcherMultiple(new LexMatcherReference("space")),
		                                          new LexMatcherReference("identifier"),
		                                          new LexMatcherMultiple(new LexMatcherReference("space")),
		                                          new LexMatcherRegex("\\("),
		                                          new LexMatcherReference("arguments"),
		                                          new LexMatcherRegex("\\)") }) });
		registerRule({ "identifier", new LexMatcherRegex("[A-Za-z_][A-Za-z0-9_]*") });
		registerRule({ "arguments",
		               new LexMatcherCombination(
		                   { new LexMatcherOptional(new LexMatcherReference("argument")),
		                     new LexMatcherMultiple(new LexMatcherReference("separatedArguments")) }) });
		registerRule({ "separatedArguments",
		               new LexMatcherBranch(
		                   { new LexMatcherCombination(
		                         { new LexMatcherMultiple(new LexMatcherReference("separation"), ELexMatcherMultipleType::OneOrMore),
		                           new LexMatcherOptional(new LexMatcherReference("argument")) }),
		                     new LexMatcherCombination(
		                         { new LexMatcherMultiple(new LexMatcherReference("separation")) }) }) });
		registerRule({ "separation",
		               new LexMatcherReference("space"), false });

		registerRule({ "argument",
		               new LexMatcherBranch(
		                   { new LexMatcherReference("bracketArgument"),
		                     new LexMatcherReference("quotedArgument"),
		                     new LexMatcherReference("unquotedArgument") }) });

		registerRule({ "bracketArgument",
		               new LexMatcherCombination(
		                   { new LexMatcherReference("bracketOpen"),
		                     new LexMatcherReference("bracketContent"),
		                     new LexMatcherReference("bracketClose") }) });
		registerRule({ "bracketOpen",
		               new LexMatcherCombination(
		                   { new LexMatcherRegex("\\["),
		                     new LexMatcherNamedGroup("BracketCount", new LexMatcherRegex("=*")) }),
		               false });
		registerRule({ "bracketContent",
		               new LexMatcherMultiple(new LexMatcherOr(
		                   { new LexMatcherRegex("."),
		                     new LexMatcherReference("newline") })),
		               false });
		registerRule({ "bracketClose",
		               new LexMatcherCombination(
		                   { new LexMatcherRegex("\\]"),
		                     new LexMatcherNamedGroupReference("BracketCount"),
		                     new LexMatcherRegex("\\]") }) });

		registerRule({ "quotedArgument",
		               new LexMatcherCombination(
		                   { new LexMatcherRegex("\\\""),
		                     new LexMatcherMultiple(new LexMatcherReference("quotedElement")),
		                     new LexMatcherRegex("\\\"") }) });
		registerRule({ "quotedElement",
		               new LexMatcherBranch(
		                   { new LexMatcherReference("escapeSequence"),
		                     new LexMatcherReference("quotedContinuation"),
		                     new LexMatcherMultiple(new LexMatcherOr(
		                         { new LexMatcherReference("newline"),
		                           new LexMatcherRegex("[^\\]") })) }) });
		registerRule({ "quotedContinuation",
		               new LexMatcherCombination(
		                   { new LexMatcherRegex("\\"),
		                     new LexMatcherReference("newline") }) });

		registerRule({ "unquotedArgument",
		               new LexMatcherBranch(
		                   { new LexMatcherMultiple(new LexMatcherReference("unquotedElement"), ELexMatcherMultipleType::OneOrMore),
		                     new LexMatcherReference("unquotedLegacy") }) });
		registerRule({ "unquotedElement",
		               new LexMatcherBranch(
		                   { new LexMatcherReference("escapeSequence"),
		                     new LexMatcherRegex("(?:[^\\s()#\\\"\\\\])+") }) });
		registerRule({ "unquotedLegacy", new LexMatcherCallback(std::bind(&Lexer::unquotedLegacyCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)) });

		registerRule({ "escapeSequence", new LexMatcherRegex("\\\\(?:[^A-Za-z0-9;]|[trn;])") });

		registerRule({ "lineComment", new LexMatcherRegex("#(?!\\[=*\\[).*") });
		registerRule({ "bracketComment",
		               new LexMatcherCombination(
		                   { new LexMatcherRegex("#"),
		                     new LexMatcherReference("bracketArgument") }) });
	}

	CommonLexer::ELexResult Lexer::unquotedLegacyCallback(CommonLexer::Lexer& lexer, CommonLexer::Lex& lex, CommonLexer::LexNode& parentNode, CommonLexer::ISource* source, const CommonLexer::SourceSpan& span)
	{
	}
} // namespace CMakeLexer