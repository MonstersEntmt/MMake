#include "Test.h"
#include "FileIO.h"

#include <CMakeLexer/Lexer.h>

#include <iostream>

static CommonLexer::FileSource s_Source("LexInput.cmake");
static CMakeLexer::Lexer s_Lexer;
static CommonLexer::Lex s_Lex;
static bool s_LexSuccess = false;

static bool testLex(Tester& tester) {
	using namespace CMakeLexer;
	
	s_Lex = s_Lexer.lexSource(&s_Source);
	if (!s_Lex.getMessages().empty()) {
		s_LexSuccess = false;
		return false;
	}
	
	if (s_Lex.getRoot().getChildren().empty()) {
		s_LexSuccess = false;
		return false;
	}
	
	s_LexSuccess = true;
	return true;
}

static bool testLineComment(Tester& tester) {
	using namespace CMakeLexer;
	
	if (!s_LexSuccess)
		return false;
	
	auto fileElement = s_Lex.getRoot().getChild(0);
	if (!fileElement)
		return false;
	
	auto lineComment = fileElement->getChild(0);
	if (!lineComment || lineComment->getType() != s_Lexer.getTLineComment())
		return false;
	
	if (s_Lex.getSource()->getSpan(lineComment->getSpan()) != " LC")
		return false;
	
	return true;
}

static bool testBracketComment(Tester& tester) {
	using namespace CMakeLexer;
	
	if (!s_LexSuccess)
		return false;
	
	auto fileElement = s_Lex.getRoot().getChild(1);
	if (!fileElement)
		return false;
	
	auto bracketComment = fileElement->getChild(0);
	if (!bracketComment || bracketComment->getType() != s_Lexer.getTBracketComment())
		return false;
	
	if (s_Lex.getSource()->getSpan(bracketComment->getSpan()) != "\nML\nC\n")
		return false;
	
	return true;
}

static bool testMultiComment(Tester& tester) {
	using namespace CMakeLexer;
	
	if (!s_LexSuccess)
		return false;
	
	auto fileElement = s_Lex.getRoot().getChild(2);
	if (!fileElement)
		return false;
	
	auto bracketComment = fileElement->getChild(0);
	if (!bracketComment || bracketComment->getType() != s_Lexer.getTBracketComment())
		return false;
	
	if (s_Lex.getSource()->getSpan(bracketComment->getSpan()) != "BC")
		return false;
	
	auto bracketComment2 = fileElement->getChild(1);
	if (!bracketComment2 || bracketComment2->getType() != s_Lexer.getTBracketComment())
		return false;
	
	if (s_Lex.getSource()->getSpan(bracketComment2->getSpan()) != "ABC")
		return false;
	
	auto lineComment = fileElement->getChild(2);
	if (!lineComment || lineComment->getType() != s_Lexer.getTLineComment())
		return false;
	
	if (s_Lex.getSource()->getSpan(lineComment->getSpan()) != "LC")
		return false;
	
	return true;
}

static bool testNoArguments(Tester& tester) {
	using namespace CMakeLexer;
	
	if (!s_LexSuccess)
		return false;
	
	auto fileElement = s_Lex.getRoot().getChild(3);
	if (!fileElement)
		return false;
	
	auto noArguments = fileElement->getChild(0);
	if (!noArguments || noArguments->getType() != s_Lexer.getTCommandInvocation() || noArguments->getChildren().empty())
		return false;
	
	auto identifier = noArguments->getChild(0);
	if (!identifier || identifier->getType() != s_Lexer.getTIdentifier() || s_Source.getSpan(identifier->getSpan()) != "no_arguments")
		return false;
	
	return true;
}

static bool testSpaceFirstNoArguments(Tester& tester) {
	using namespace CMakeLexer;
	
	if (!s_LexSuccess)
		return false;
	
	auto fileElement = s_Lex.getRoot().getChild(4);
	if (!fileElement)
		return false;
	
	auto spaceFirstNoArguments = fileElement->getChild(0);
	if (!spaceFirstNoArguments || spaceFirstNoArguments->getType() != s_Lexer.getTCommandInvocation() || spaceFirstNoArguments->getChildren().empty())
		return false;
	
	auto identifier = spaceFirstNoArguments->getChild(0);
	if (!identifier || identifier->getType() != s_Lexer.getTIdentifier() || s_Source.getSpan(identifier->getSpan()) != "space_first_no_arguments")
		return false;
	
	return true;
}

static bool testSpaceAfterNoArguments(Tester& tester) {
	using namespace CMakeLexer;
	
	if (!s_LexSuccess)
		return false;
	
	auto fileElement = s_Lex.getRoot().getChild(5);
	if (!fileElement)
		return false;
	
	auto spaceAfterNoArguments = fileElement->getChild(0);
	if (!spaceAfterNoArguments || spaceAfterNoArguments->getType() != s_Lexer.getTCommandInvocation() || spaceAfterNoArguments->getChildren().empty())
		return false;
	
	auto identifier = spaceAfterNoArguments->getChild(0);
	if (!identifier || identifier->getType() != s_Lexer.getTIdentifier() || s_Source.getSpan(identifier->getSpan()) != "space_after_no_arguments")
		return false;
	
	return true;
}

static bool testBracketArgument(Tester& tester) {
	using namespace CMakeLexer;
	
	if (!s_LexSuccess)
		return false;
	
	auto fileElement = s_Lex.getRoot().getChild(6);
	if (!fileElement)
		return false;
	
	auto bracketArgument = fileElement->getChild(0);
	if (!bracketArgument || bracketArgument->getType() != s_Lexer.getTCommandInvocation() || bracketArgument->getChildren().size() != 2)
		return false;
	
	auto identifier = bracketArgument->getChild(0);
	if (!identifier || identifier->getType() != s_Lexer.getTIdentifier() || s_Source.getSpan(identifier->getSpan()) != "bracket_argument")
		return false;
	
	auto arguments = bracketArgument->getChild(1);
	if (!arguments || arguments->getType() != s_Lexer.getTArguments() || arguments->getChildren().empty())
		return false;
	
	auto arg1 = arguments->getChild(0);
	if (!arg1 || arg1->getType() != s_Lexer.getTBracketContent() || s_Source.getSpan(arg1->getSpan()) != "arg1\n${VAR}")
		return false;
	
	return true;
}

static bool testQuotedArgument(Tester& tester) {
	using namespace CMakeLexer;
	
	if (!s_LexSuccess)
		return false;
	
	auto fileElement = s_Lex.getRoot().getChild(7);
	if (!fileElement)
		return false;
	
	auto quotedArgument = fileElement->getChild(0);
	if (!quotedArgument || quotedArgument->getType() != s_Lexer.getTCommandInvocation() || quotedArgument->getChildren().size() != 2)
		return false;
	
	auto identifier = quotedArgument->getChild(0);
	if (!identifier || identifier->getType() != s_Lexer.getTIdentifier() || s_Source.getSpan(identifier->getSpan()) != "quoted_argument")
		return false;
	
	auto arguments = quotedArgument->getChild(1);
	if (!arguments || arguments->getType() != s_Lexer.getTArguments() || arguments->getChildren().empty())
		return false;
	
	auto arg1 = arguments->getChild(0);
	if (!arg1 || arg1->getType() != s_Lexer.getTQuotedArgument() || arg1->getChildren().empty())
		return false;
	
	auto arg1el = arg1->getChild(0);
	if (!arg1el || arg1el->getType() != s_Lexer.getTQuotedElement() || s_Source.getSpan(arg1el->getSpan()) != "arg1 ${VAR}")
		return false;
	
	return true;
}

static bool testUnquotedArgument(Tester& tester) {
	using namespace CMakeLexer;
	
	if (!s_LexSuccess)
		return false;
	
	auto fileElement = s_Lex.getRoot().getChild(8);
	if (!fileElement)
		return false;
	
	auto unquotedArgument = fileElement->getChild(0);
	if (!unquotedArgument || unquotedArgument->getType() != s_Lexer.getTCommandInvocation() || unquotedArgument->getChildren().size() != 2)
		return false;
	
	auto identifier = unquotedArgument->getChild(0);
	if (!identifier || identifier->getType() != s_Lexer.getTIdentifier() || s_Source.getSpan(identifier->getSpan()) != "unquoted_argument")
		return false;
	
	auto arguments = unquotedArgument->getChild(1);
	if (!arguments || arguments->getType() != s_Lexer.getTArguments() || arguments->getChildren().empty())
		return false;
	
	auto arg1 = arguments->getChild(0);
	if (!arg1 || arg1->getType() != s_Lexer.getTUnquotedArgument() || arg1->getChildren().empty())
		return false;
	
	auto arg1el = arg1->getChild(0);
	if (!arg1el || arg1el->getType() != s_Lexer.getTUnquotedElement() || s_Source.getSpan(arg1el->getSpan()) != "arg1${VAR}")
		return false;
	
	return true;
}
	
static bool testUnquotedLegacyArgument(Tester& tester) {
	using namespace CMakeLexer;
	
	if (!s_LexSuccess)
		return false;
	
	auto fileElement = s_Lex.getRoot().getChild(9);
	if (!fileElement)
		return false;
	
	auto unquotedLegacyArgument = fileElement->getChild(0);
	if (!unquotedLegacyArgument || unquotedLegacyArgument->getType() != s_Lexer.getTCommandInvocation() || unquotedLegacyArgument->getChildren().size() != 2)
		return false;
	
	auto identifier = unquotedLegacyArgument->getChild(0);
	if (!identifier || identifier->getType() != s_Lexer.getTIdentifier() || s_Source.getSpan(identifier->getSpan()) != "unquoted_legacy_argument")
		return false;
	
	auto arguments = unquotedLegacyArgument->getChild(1);
	if (!arguments || arguments->getType() != s_Lexer.getTArguments() || arguments->getChildren().empty())
		return false;
	
	auto arg1 = arguments->getChild(0);
	if (!arg1 || arg1->getType() != s_Lexer.getTUnquotedLegacy() || s_Source.getSpan(arg1->getSpan()) != "arg=\"1\"")
		return false;
	
	return true;
}

static bool testUnquotedLegacyArgument2(Tester& tester) {
	using namespace CMakeLexer;
	
	if (!s_LexSuccess)
		return false;
	
	auto fileElement = s_Lex.getRoot().getChild(10);
	if (!fileElement)
		return false;
	
	auto unquotedLegacyArgument2 = fileElement->getChild(0);
	if (!unquotedLegacyArgument2 || unquotedLegacyArgument2->getType() != s_Lexer.getTCommandInvocation() || unquotedLegacyArgument2->getChildren().size() != 2)
		return false;
	
	auto identifier = unquotedLegacyArgument2->getChild(0);
	if (!identifier || identifier->getType() != s_Lexer.getTIdentifier() || s_Source.getSpan(identifier->getSpan()) != "unquoted_legacy_argument2")
		return false;
	
	auto arguments = unquotedLegacyArgument2->getChild(1);
	if (!arguments || arguments->getType() != s_Lexer.getTArguments() || arguments->getChildren().empty())
		return false;
	
	auto arg1 = arguments->getChild(0);
	if (!arg1 || arg1->getType() != s_Lexer.getTUnquotedLegacy() || s_Source.getSpan(arg1->getSpan()) != "arg=$(1)")
		return false;
	
	return true;
}

static bool testSpaceBeforeArgument(Tester& tester) {
	using namespace CMakeLexer;
	
	if (!s_LexSuccess)
		return false;
	
	auto fileElement = s_Lex.getRoot().getChild(11);
	if (!fileElement)
		return false;
	
	auto spaceBeforeArgument = fileElement->getChild(0);
	if (!spaceBeforeArgument || spaceBeforeArgument->getType() != s_Lexer.getTCommandInvocation() || spaceBeforeArgument->getChildren().size() != 2)
		return false;
	
	auto identifier = spaceBeforeArgument->getChild(0);
	if (!identifier || identifier->getType() != s_Lexer.getTIdentifier() || s_Source.getSpan(identifier->getSpan()) != "space_before_argument")
		return false;
	
	auto arguments = spaceBeforeArgument->getChild(1);
	if (!arguments || arguments->getType() != s_Lexer.getTArguments() || arguments->getChildren().empty())
		return false;
	
	auto arg1 = arguments->getChild(0);
	if (!arg1 || arg1->getType() != s_Lexer.getTUnquotedArgument() || arg1->getChildren().empty())
		return false;
	
	auto arg1el = arg1->getChild(0);
	if (!arg1el || arg1el->getType() != s_Lexer.getTUnquotedElement() || s_Source.getSpan(arg1el->getSpan()) != "arg1")
		return false;
	
	return true;
}

static bool testSpaceAfterArgument(Tester& tester) {
	using namespace CMakeLexer;
	
	if (!s_LexSuccess)
		return false;
	
	auto fileElement = s_Lex.getRoot().getChild(12);
	if (!fileElement)
		return false;
	
	auto spaceAfterArgument = fileElement->getChild(0);
	if (!spaceAfterArgument || spaceAfterArgument->getType() != s_Lexer.getTCommandInvocation() || spaceAfterArgument->getChildren().size() != 2)
		return false;
	
	auto identifier = spaceAfterArgument->getChild(0);
	if (!identifier || identifier->getType() != s_Lexer.getTIdentifier() || s_Source.getSpan(identifier->getSpan()) != "space_after_argument")
		return false;
	
	auto arguments = spaceAfterArgument->getChild(1);
	if (!arguments || arguments->getType() != s_Lexer.getTArguments() || arguments->getChildren().empty())
		return false;
	
	auto arg1 = arguments->getChild(0);
	if (!arg1 || arg1->getType() != s_Lexer.getTUnquotedArgument() || arg1->getChildren().empty())
		return false;
	
	auto arg1el = arg1->getChild(0);
	if (!arg1el || arg1el->getType() != s_Lexer.getTUnquotedElement() || s_Source.getSpan(arg1el->getSpan()) != "arg1")
		return false;
	
	return true;
}

static bool testMultiArgument(Tester& tester) {
	using namespace CMakeLexer;
	
	if (!s_LexSuccess)
		return false;
	
	auto fileElement = s_Lex.getRoot().getChild(13);
	if (!fileElement)
		return false;
	
	auto multiArgument = fileElement->getChild(0);
	if (!multiArgument || multiArgument->getType() != s_Lexer.getTCommandInvocation() || multiArgument->getChildren().size() != 2)
		return false;
	
	auto identifier = multiArgument->getChild(0);
	if (!identifier || identifier->getType() != s_Lexer.getTIdentifier() || s_Source.getSpan(identifier->getSpan()) != "multi_argument")
		return false;
	
	auto arguments = multiArgument->getChild(1);
	if (!arguments || arguments->getType() != s_Lexer.getTArguments() || arguments->getChildren().size() != 3)
		return false;
	
	auto arg1 = arguments->getChild(0);
	if (!arg1 || arg1->getType() != s_Lexer.getTBracketContent() || s_Source.getSpan(arg1->getSpan()) != "arg1")
		return false;
	
	auto arg2 = arguments->getChild(1);
	if (!arg2 || arg2->getType() != s_Lexer.getTQuotedArgument() || arg2->getChildren().empty())
		return false;
	
	auto arg2el = arg2->getChild(0);
	if (!arg2el || arg2el->getType() != s_Lexer.getTQuotedElement() || s_Source.getSpan(arg2el->getSpan()) != "arg2")
		return false;
	
	auto arg3 = arguments->getChild(2);
	if (!arg3 || arg3->getType() != s_Lexer.getTUnquotedArgument() || arg3->getChildren().empty())
		return false;
	
	auto arg3el = arg3->getChild(0);
	if (!arg3el || arg3el->getType() != s_Lexer.getTUnquotedElement() || s_Source.getSpan(arg3el->getSpan()) != "arg3")
		return false;
	
	return true;
}

static bool testMultilineCommand(Tester& tester) {
	using namespace CMakeLexer;
	
	if (!s_LexSuccess)
		return false;
	
	auto fileElement = s_Lex.getRoot().getChild(14);
	if (!fileElement)
		return false;
	
	auto multilineCommand = fileElement->getChild(0);
	if (!multilineCommand || multilineCommand->getType() != s_Lexer.getTCommandInvocation() || multilineCommand->getChildren().size() != 2)
		return false;
	
	auto identifier = multilineCommand->getChild(0);
	if (!identifier || identifier->getType() != s_Lexer.getTIdentifier() || s_Source.getSpan(identifier->getSpan()) != "multiline_command")
		return false;
	
	auto arguments = multilineCommand->getChild(1);
	if (!arguments || arguments->getType() != s_Lexer.getTArguments() || arguments->getChildren().size() != 2)
		return false;
	
	auto arg1 = arguments->getChild(0);
	if (!arg1 || arg1->getType() != s_Lexer.getTUnquotedArgument() || arg1->getChildren().empty())
		return false;
	
	auto arg1el = arg1->getChild(0);
	if (!arg1el || arg1el->getType() != s_Lexer.getTUnquotedElement() || s_Source.getSpan(arg1el->getSpan()) != "arg1")
		return false;
	
	auto arg2 = arguments->getChild(1);
	if (!arg2 || arg2->getType() != s_Lexer.getTUnquotedArgument() || arg2->getChildren().empty())
		return false;
	
	auto arg2el = arg2->getChild(0);
	if (!arg2el || arg2el->getType() != s_Lexer.getTUnquotedElement() || s_Source.getSpan(arg2el->getSpan()) != "arg2")
		return false;
	
	return true;
}

static bool testArgumentsInArguments(Tester& tester) {
	using namespace CMakeLexer;
	
	if (!s_LexSuccess)
		return false;
	
	auto fileElement = s_Lex.getRoot().getChild(15);
	if (!fileElement)
		return false;
	
	auto multiArgument = fileElement->getChild(0);
	if (!multiArgument || multiArgument->getType() != s_Lexer.getTCommandInvocation() || multiArgument->getChildren().size() != 2)
		return false;
	
	auto identifier = multiArgument->getChild(0);
	if (!identifier || identifier->getType() != s_Lexer.getTIdentifier() || s_Source.getSpan(identifier->getSpan()) != "multi_argument")
		return false;
	
	auto arguments = multiArgument->getChild(1);
	if (!arguments || arguments->getType() != s_Lexer.getTArguments() || arguments->getChildren().empty())
		return false;
	
	auto args = arguments->getChild(0);
	if (!args || args->getType() != s_Lexer.getTArguments() || arguments->getChildren().size() != 3)
		return false;
	
	auto arg1 = args->getChild(0);
	if (!arg1 || arg1->getType() != s_Lexer.getTBracketContent() || s_Source.getSpan(arg1->getSpan()) != "arg1")
		return false;
	
	auto arg2 = args->getChild(1);
	if (!arg2 || arg2->getType() != s_Lexer.getTQuotedArgument() || arg2->getChildren().empty())
		return false;
	
	auto arg2el = arg2->getChild(0);
	if (!arg2el || arg2el->getType() != s_Lexer.getTQuotedElement() || s_Source.getSpan(arg2el->getSpan()) != "arg2")
		return false;
	
	auto arg3 = args->getChild(2);
	if (!arg3 || arg3->getType() != s_Lexer.getTUnquotedArgument() || arg3->getChildren().empty())
		return false;
	
	auto arg3el = arg3->getChild(0);
	if (!arg3el || arg3el->getType() != s_Lexer.getTUnquotedElement() || s_Source.getSpan(arg3el->getSpan()) != "arg3")
		return false;
	
	return true;
}

static bool testEscapeCharacters(Tester& tester) {
	using namespace CMakeLexer;
	
	if (!s_LexSuccess)
		return false;
	
	auto fileElement = s_Lex.getRoot().getChild(14);
	if (!fileElement)
		return false;
	
	auto escapeCharacters = fileElement->getChild(0);
	if (!escapeCharacters || escapeCharacters->getType() != s_Lexer.getTCommandInvocation() || escapeCharacters->getChildren().size() != 2)
		return false;
	
	auto identifier = escapeCharacters->getChild(0);
	if (!identifier || identifier->getType() != s_Lexer.getTIdentifier() || s_Source.getSpan(identifier->getSpan()) != "escape_characters")
		return false;
	
	auto arguments = escapeCharacters->getChild(1);
	if (!arguments || arguments->getType() != s_Lexer.getTArguments() || arguments->getChildren().size() != 2)
		return false;
	
	auto arg1 = arguments->getChild(0);
	if (!arg1 || arg1->getType() != s_Lexer.getTQuotedArgument() || arg1->getChildren().size() != 4)
		return false;
	
	auto arg1el = arg1->getChild(0);
	if (!arg1el || arg1el->getType() != s_Lexer.getTEscapeSequence() || s_Source.getSpan(arg1el->getSpan()) != "r")
		return false;
	
	auto arg1el2 = arg1->getChild(1);
	if (!arg1el2 || arg1el2->getType() != s_Lexer.getTEscapeSequence() || s_Source.getSpan(arg1el2->getSpan()) != "t")
		return false;
	
	auto arg1el3 = arg1->getChild(2);
	if (!arg1el3 || arg1el3->getType() != s_Lexer.getTEscapeSequence() || s_Source.getSpan(arg1el3->getSpan()) != "n")
		return false;
	
	auto arg1el4 = arg1->getChild(4);
	if (!arg1el4 || arg1el4->getType() != s_Lexer.getTEscapeSequence() || s_Source.getSpan(arg1el4->getSpan()) != ";")
		return false;
	
	auto arg2 = arguments->getChild(1);
	if (!arg2 || arg2->getType() != s_Lexer.getTUnquotedArgument() || arg2->getChildren().empty())
		return false;
	
	auto arg2el = arg2->getChild(0);
	if (!arg2el || arg2el->getType() != s_Lexer.getTEscapeSequence() || s_Source.getSpan(arg2el->getSpan()) != ")")
		return false;
	
	auto arg2el2 = arg2->getChild(1);
	if (!arg2el2 || arg2el2->getType() != s_Lexer.getTUnquotedElement() || s_Source.getSpan(arg2el2->getSpan()) != "a")
		return false;
	
	return true;
}

struct LexerTestsRegister {
	LexerTestsRegister() {
		auto& tester = Tester::Get();
		tester.addTest("CMakeLexer", "Lex", &testLex);
		tester.addTest("CMakeLexer", "LineComment", &testLineComment);
		tester.addTest("CMakeLexer", "BracketComment", &testBracketComment);
		tester.addTest("CMakeLexer", "MultiComment", &testMultiComment);
		tester.addTest("CMakeLexer", "NoArguments", &testNoArguments);
		tester.addTest("CMakeLexer", "SpaceFirstNoArguments", &testSpaceFirstNoArguments);
		tester.addTest("CMakeLexer", "SpaceAfterNoArguments", &testSpaceAfterNoArguments);
		tester.addTest("CMakeLexer", "BracketArgument", &testBracketArgument);
		tester.addTest("CMakeLexer", "QuotedArgument", &testQuotedArgument);
		tester.addTest("CMakeLexer", "UnquotedArgument", &testUnquotedArgument);
		tester.addTest("CMakeLexer", "UnquotedLegacyArgument", &testUnquotedLegacyArgument);
		tester.addTest("CMakeLexer", "UnquotedLegacyArgument2", &testUnquotedLegacyArgument2);
		tester.addTest("CMakeLexer", "SpaceBeforeArgument", &testSpaceBeforeArgument);
		tester.addTest("CMakeLexer", "SpaceAfterArgument", &testSpaceAfterArgument);
		tester.addTest("CMakeLexer", "MultiArgument", &testMultiArgument);
		tester.addTest("CMakeLexer", "MultilineCommand", &testMultilineCommand);
		tester.addTest("CMakeLexer", "ArgumentsInArguments", &testArgumentsInArguments);
		tester.addTest("CMakeLexer", "EscapeCharacters", &testEscapeCharacters);
	}
} lexerTestsRegister;
