#include "CMakeLexer/Lexer.h"

namespace CMakeLexer
{
	Lexer::Lexer()
	{
		using namespace CommonLexer;
		setMainRule("File");

		registerRule({ "File", new LexMatcherMultiple(new LexMatcherReference("FileElement")) });
		registerRule({ "FileElement",
		               new LexMatcherBranch(
		                   { new LexMatcherCombination(
		                         { new LexMatcherReference("CommandInvocation"),
		                           new LexMatcherReference("LineEnding") }),
		                     new LexMatcherCombination(
		                         { new LexMatcherMultiple(new LexMatcherOr(
		                               { new LexMatcherReference("BracketComment"),
		                                 new LexMatcherReference("Space") })),
		                           new LexMatcherReference("LineEnding") }) }) });
		registerRule({ "LineEnding",
		               new LexMatcherCombination(
		                   { new LexMatcherOptional(new LexMatcherReference("LineComment")),
		                     new LexMatcherReference("Newline") }),
		               false });
		registerRule({ "Space", new LexMatcherRegex("[ \t]+"), false });
		registerRule({ "Newline", new LexMatcherRegex("\n"), false });

		registerRule({ "CommandInvocation", new LexMatcherCombination(
		                                        { new LexMatcherMultiple(new LexMatcherReference("Space")),
		                                          new LexMatcherReference("Identifier"),
		                                          new LexMatcherMultiple(new LexMatcherReference("Space")),
		                                          new LexMatcherRegex("\\("),
		                                          new LexMatcherReference("Arguments"),
		                                          new LexMatcherRegex("\\)") }) });
		registerRule({ "Identifier", new LexMatcherRegex("[A-Za-z_][A-Za-z0-9_]*") });
		registerRule({ "Arguments",
		               new LexMatcherCombination(
		                   { new LexMatcherOptional(new LexMatcherReference("Argument")),
		                     new LexMatcherMultiple(new LexMatcherReference("SeparatedArguments")) }) });
		registerRule({ "SeparatedArguments",
		               new LexMatcherBranch(
		                   { new LexMatcherCombination(
		                         { new LexMatcherMultiple(new LexMatcherReference("Separation"), ELexMatcherMultipleType::OneOrMore),
		                           new LexMatcherOptional(new LexMatcherReference("Argument")) }),
		                     new LexMatcherCombination(
		                         { new LexMatcherMultiple(new LexMatcherReference("Separation")),
		                           new LexMatcherRegex("\\("),
		                           new LexMatcherReference("Arguments"),
		                           new LexMatcherRegex("\\)") }) }) });
		registerRule({ "Separation",
		               new LexMatcherReference("Space"), false });

		registerRule({ "Argument",
		               new LexMatcherBranch(
		                   { new LexMatcherReference("BracketArgument"),
		                     new LexMatcherReference("QuotedArgument"),
		                     new LexMatcherReference("UnquotedArgument") }) });

		registerRule({ "BracketArgument",
		               new LexMatcherCombination(
		                   { new LexMatcherReference("BracketOpen"),
		                     new LexMatcherReference("BracketContent"),
		                     new LexMatcherReference("BracketClose") }) });
		registerRule({ "BracketOpen",
		               new LexMatcherCombination(
		                   { new LexMatcherRegex("\\["),
		                     new LexMatcherNamedGroup("BracketCount", new LexMatcherRegex("=*")) }),
		               false });
		registerRule({ "BracketContent",
		               new LexMatcherMultiple(new LexMatcherOr(
		                   { new LexMatcherRegex("."),
		                     new LexMatcherReference("Newline") })),
		               false });
		registerRule({ "BracketClose",
		               new LexMatcherCombination(
		                   { new LexMatcherRegex("\\]"),
		                     new LexMatcherNamedGroupReference("BracketCount"),
		                     new LexMatcherRegex("\\]") }) });

		registerRule({ "QuotedArgument",
		               new LexMatcherCombination(
		                   { new LexMatcherRegex("\\\""),
		                     new LexMatcherMultiple(new LexMatcherReference("QuotedElement")),
		                     new LexMatcherRegex("\\\"") }) });
		registerRule({ "QuotedElement",
		               new LexMatcherBranch(
		                   { new LexMatcherReference("EscapeSequence"),
		                     new LexMatcherReference("QuotedContinuation"),
		                     new LexMatcherMultiple(
		                         new LexMatcherOr(
		                             { new LexMatcherReference("Newline"),
		                               new LexMatcherRegex("[^\"\\\\]") }),
		                         ELexMatcherMultipleType::OneOrMore) }) });
		registerRule({ "QuotedContinuation",
		               new LexMatcherCombination(
		                   { new LexMatcherRegex("\\\\"),
		                     new LexMatcherReference("Newline") }) });

		registerRule({ "UnquotedArgument",
		               new LexMatcherBranch(
		                   { new LexMatcherMultiple(new LexMatcherReference("UnquotedElement"), ELexMatcherMultipleType::OneOrMore),
		                     new LexMatcherReference("UnquotedLegacy") }) });
		registerRule({ "UnquotedElement",
		               new LexMatcherBranch(
		                   { new LexMatcherReference("EscapeSequence"),
		                     new LexMatcherRegex("(?:[^\\s()#\\\"\\\\])+") }) });
		registerRule({ "UnquotedLegacy", new LexMatcherCallback(std::bind(&Lexer::unquotedLegacyCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)) });

		registerRule({ "EscapeSequence", new LexMatcherRegex("\\\\(?:[^A-Za-z0-9;]|[trn;])") });

		registerRule({ "LineComment", new LexMatcherRegex("#(?!\\[=*\\[).*") });
		registerRule({ "BracketComment",
		               new LexMatcherCombination(
		                   { new LexMatcherRegex("#"),
		                     new LexMatcherReference("BracketArgument") }) });
	}

	CommonLexer::LexResult Lexer::unquotedLegacyCallback([[maybe_unused]] CommonLexer::Lex& lex, [[maybe_unused]] CommonLexer::LexNode& parentNode, [[maybe_unused]] CommonLexer::ISource* source, [[maybe_unused]] const CommonLexer::SourceSpan& span)
	{
		auto itr = span.begin(source);
		auto end = span.end(source);
		if (itr == end || *itr == '"' || *itr == '(')
			return CommonLexer::ELexStatus::Failure;

		bool insideString = false;

		bool        escaped  = false;
		bool        breakOut = false;
		std::size_t len      = 0;
		while (itr != end)
		{
			char c = *itr;

			switch (c)
			{
			case '\\':
				escaped = true;
				++itr;
				++len;
				break;
			case '"':
				if (!escaped)
					insideString = !insideString;
				++itr;
				++len;
				break;
			case '$':
				++itr;
				++len;
				if (itr == end)
					break;

				if (!escaped)
				{
					if (*itr == '(')
					{
						++itr;
						++len;
						while (itr != end)
						{
							if (*itr == ')')
								break;
							++itr;
							++len;
						}
						++itr;
						++len;
					}
				}
				break;
			case ')': [[fallthrough]];
			case '\n':
				breakOut = true;
				break;
			case ' ':
				if (!escaped && !insideString)
				{
					breakOut = true;
					break;
				}
				++itr;
				++len;
				break;
			default:
				escaped = false;
				++itr;
				++len;
			}

			if (breakOut)
				break;
		}

		if (len == 0)
			return CommonLexer::ELexStatus::Failure;

		return { CommonLexer::ELexStatus::Success, { span.m_Begin, itr } };
	}
} // namespace CMakeLexer