#include "CommonLexer/LexerLexer.h"

namespace CommonLexer
{
	LexerLexer::LexerLexer()
	{
		setMainRule("File");

		registerRule({ "File", new LexMatcherMultiple(new LexMatcherReference("LineElement")), false });
		registerRule({ "LineElement",
		               new LexMatcherCombination(
		                   { new LexMatcherOptional(new LexMatcherReference("Line")),
		                     new LexMatcherRegex("([ \t]*#.*)?[ \t]*\n?") }),
		               false });
		registerRule({ "Line",
		               new LexMatcherBranch(
		                   { new LexMatcherReference("NodeRule"),
		                     new LexMatcherReference("NonNodeRule"),
		                     new LexMatcherReference("CallbackRule") }) });
		registerRule({ "NodeRule",
		               new LexMatcherCombination(
		                   { new LexMatcherReference("Identifier"),
		                     new LexMatcherRegex("[ \t]*:[ \t]*"),
		                     new LexMatcherReference("Value") }) });
		registerRule({ "NonNodeRule",
		               new LexMatcherCombination(
		                   { new LexMatcherReference("Identifier"),
		                     new LexMatcherRegex("[ \t]*\\?[ \t]*:[ \t]*"),
		                     new LexMatcherReference("Value") }) });
		registerRule({ "CallbackRule",
		               new LexMatcherCombination(
		                   { new LexMatcherReference("Identifier"),
		                     new LexMatcherRegex("[ \t]*\\![ \t]*:") }) });
		registerRule({ "Identifier", new LexMatcherRegex("[A-Za-z_][A-Za-z0-9_]*") });
		registerRule({ "Value",
		               new LexMatcherBranch(
		                   { new LexMatcherReference("Branch"),
		                     new LexMatcherReference("OneLineValue") }),
		               false });
		registerRule({ "OneLineValue",
		               new LexMatcherBranch(
		                   { new LexMatcherReference("Combination"),
		                     new LexMatcherReference("Or"),
		                     new LexMatcherReference("NonMultValue") }),
		               false });
		registerRule({ "NonMultValue",
		               new LexMatcherBranch(
		                   { new LexMatcherReference("ZeroOrMore"),
		                     new LexMatcherReference("OneOrMore"),
		                     new LexMatcherReference("Optional"),
		                     new LexMatcherReference("BasicValue") }),
		               false });
		registerRule({ "BasicValue",
		               new LexMatcherBranch(
		                   { new LexMatcherReference("Group"),
		                     new LexMatcherReference("NamedGroup"),
		                     new LexMatcherReference("Reference"),
		                     new LexMatcherReference("Identifier"),
		                     new LexMatcherReference("Regex") }),
		               false });
		registerRule({ "Regex", new LexMatcherRegex("\\'(?:[^\\'\\\\\n]|\\.|\\\\.)*\\'") });
		registerRule({ "ZeroOrMore",
		               new LexMatcherCombination(
		                   { new LexMatcherReference("BasicValue"),
		                     new LexMatcherRegex("\\*") }) });
		registerRule({ "OneOrMore",
		               new LexMatcherCombination(
		                   { new LexMatcherReference("BasicValue"),
		                     new LexMatcherRegex("\\+") }) });
		registerRule({ "Optional",
		               new LexMatcherCombination(
		                   { new LexMatcherReference("BasicValue"),
		                     new LexMatcherRegex("\\?") }) });
		registerRule({ "Combination",
		               new LexMatcherCombination(
		                   { new LexMatcherReference("NonMultValue"),
		                     new LexMatcherMultiple(
		                         { new LexMatcherCombination(
		                             { new LexMatcherRegex("[ \t]+"),
		                               new LexMatcherReference("NonMultValue") }) },
		                         ELexMatcherMultipleType::OneOrMore) }) });
		registerRule({ "Or",
		               new LexMatcherCombination(
		                   { new LexMatcherReference("NonMultValue"),
		                     new LexMatcherMultiple(
		                         { new LexMatcherCombination(
		                             { new LexMatcherRegex("[ \t]*\\|[ \t]*"),
		                               new LexMatcherReference("NonMultValue") }) },
		                         ELexMatcherMultipleType::OneOrMore) }) });
		registerRule({ "Branch",
		               new LexMatcherCombination(
		                   { new LexMatcherReference("OneLineValue"),
		                     new LexMatcherMultiple(
		                         { new LexMatcherCombination(
		                             { new LexMatcherRegex("\n[ \t]+"),
		                               new LexMatcherReference("OneLineValue") }) },
		                         ELexMatcherMultipleType::OneOrMore) }) });
		registerRule({ "Group",
		               new LexMatcherCombination(
		                   { new LexMatcherRegex("\\([ \t]*"),
		                     new LexMatcherReference("OneLineValue"),
		                     new LexMatcherRegex("[ \t]*\\)") }) });
		registerRule({ "NamedGroup",
		               new LexMatcherCombination(
		                   { new LexMatcherRegex("\\([ \t]*<[ \t]*"),
		                     new LexMatcherReference("Identifier"),
		                     new LexMatcherRegex("[ \t]*>[ \t]*:[ \t]*"),
		                     new LexMatcherReference("OneLineValue"),
		                     new LexMatcherRegex("[ \t]*\\)") }) });
		registerRule({ "Reference",
		               new LexMatcherCombination(
		                   { new LexMatcherRegex("\\\\"),
		                     new LexMatcherReference("Identifier") }) });
	}
} // namespace CommonLexer