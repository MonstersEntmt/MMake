#include "CMakeLexer/Lexer.h"

namespace CMakeLexer
{
	Lexer::Lexer()
	{
		using namespace CommonLexer;
		setMainRule("File");

		/*registerRule({ "File", MatcherMultiple(MatcherReference("FileElement")) });
		registerRule({ "FileElement",
		               MatcherBranch(Tuple {
		                   MatcherCombination(Tuple {
		                       MatcherReference("CommandInvocation"),
		                       MatcherReference("LineEnding") }),
		                   MatcherCombination(Tuple {
		                       MatcherMultiple(
		                           MatcherOr(Tuple {
		                               MatcherReference("BracketComment"),
		                               MatcherReference("Space") })),
		                       MatcherReference("LineEnding") }) }) });
		registerRule({ "LineEnding",
		               MatcherCombination(Tuple {
		                   MatcherOptional(MatcherReference("LineComment")),
		                   MatcherReference("Newline") }),
		               false });
		registerRule({ "Space", MatcherRegex("[ \t]+"), false });
		registerRule({ "Newline", MatcherText("\n"), false });

		registerRule({ "CommandInvocation", MatcherCombination(Tuple {
		                                        MatcherMultiple(MatcherReference("Space")),
		                                        MatcherReference("Identifier"),
		                                        MatcherMultiple(MatcherReference("Space")),
		                                        MatcherText("("),
		                                        MatcherReference("Arguments"),
		                                        MatcherText(")") }) });
		registerRule({ "Identifier", MatcherRegex("[A-Za-z_][A-Za-z0-9_]*") });
		registerRule({ "Arguments",
		               MatcherCombination(Tuple {
		                   MatcherOptional(MatcherReference("Argument")),
		                   MatcherMultiple(MatcherReference("SeparatedArguments")) }) });
		registerRule({ "SeparatedArguments",
		               MatcherBranch(Tuple {
		                   MatcherCombination(Tuple {
		                       MatcherMultiple(MatcherReference("Separation"), EMatcherMultipleType::OneOrMore),
		                       MatcherOptional(MatcherReference("Argument")) }),
		                   MatcherCombination(Tuple {
		                       MatcherMultiple(MatcherReference("Separation")),
		                       MatcherText("("),
		                       MatcherReference("Arguments"),
		                       MatcherText(")") }) }) });
		registerRule({ "Separation", MatcherReference("Space"), false });

		registerRule({ "Argument",
		               MatcherBranch(Tuple {
		                   MatcherReference("BracketArgument"),
		                   MatcherReference("QuotedArgument"),
		                   MatcherReference("UnquotedArgument") }) });

		registerRule({ "BracketArgument",
		               MatcherCombination(Tuple {
		                   MatcherReference("BracketOpen"),
		                   MatcherReference("BracketContent"),
		                   MatcherReference("BracketClose") }) });
		registerRule({ "BracketOpen",
		               MatcherCombination(Tuple {
		                   MatcherText("["),
		                   MatcherNamedGroup("BracketCount", MatcherRegex("=*")) }),
		               false });
		registerRule({ "BracketContent",
		               MatcherMultiple(
		                   MatcherOr(Tuple {
		                       MatcherRegex("."),
		                       MatcherReference("Newline") })),
		               false });
		registerRule({ "BracketClose",
		               MatcherCombination(Tuple {
		                   MatcherText("]"),
		                   MatcherNamedGroupReference("BracketCount"),
		                   MatcherText("]") }) });

		registerRule({ "QuotedArgument",
		               MatcherCombination(Tuple {
		                   MatcherText("\""),
		                   MatcherMultiple(MatcherReference("QuotedElement")),
		                   MatcherText("\"") }) });
		registerRule({ "QuotedElement",
		               MatcherBranch(Tuple {
		                   MatcherReference("EscapeSequence"),
		                   MatcherReference("QuotedContinuation"),
		                   MatcherMultiple(
		                       MatcherOr(Tuple {
		                           MatcherReference("Newline"),
		                           MatcherRegex("[^\"\\\\]") }),
		                       EMatcherMultipleType::OneOrMore) }) });
		registerRule({ "QuotedContinuation",
		               MatcherCombination(Tuple {
		                   MatcherText("\\"),
		                   MatcherReference("Newline") }) });

		registerRule({ "UnquotedArgument",
		               MatcherBranch(Tuple {
		                   MatcherMultiple(MatcherReference("UnquotedElement"), EMatcherMultipleType::OneOrMore),
		                   MatcherReference("UnquotedLegacy") }) });
		registerRule({ "UnquotedElement",
		               MatcherBranch(Tuple {
		                   MatcherReference("EscapeSequence"),
		                   MatcherRegex("(?:[^\\s()#\\\"\\\\])+") }) });
		registerRule({ "UnquotedLegacy", MatcherCallback(std::bind(&Lexer::unquotedLegacyCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)) });

		registerRule({ "EscapeSequence", MatcherRegex("\\\\(?:[^A-Za-z0-9;]|[trn;])") });

		registerRule({ "LineComment", MatcherRegex("#(?!\\[=*\\[).*") });
		registerRule({ "BracketComment",
		               MatcherCombination(Tuple {
		                   MatcherText("#"),
		                   MatcherReference("BracketArgument") }) });*/
	}

	CommonLexer::MatchResult Lexer::unquotedLegacyCallback(CommonLexer::MatcherState& state, CommonLexer::SourceSpan span)
	{
		auto itr = span.begin(state.m_Source);
		auto end = span.end(state.m_Source);
		if (itr == end || *itr == '"' || *itr == '(')
			return { CommonLexer::EMatchStatus::Failure, { span.m_Begin, span.m_Begin } };

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
			return { CommonLexer::EMatchStatus::Failure, { span.m_Begin, itr } };

		return { CommonLexer::EMatchStatus::Success, { span.m_Begin, itr } };
	}
} // namespace CMakeLexer