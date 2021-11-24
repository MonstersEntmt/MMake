#include "Test.h"
#include "FileIO.h"

#include <CMakeInterpreter/Lexer.h>

#include <iostream>

static CMakeInterpreter::Lex s_Lex;
static bool s_LexSuccess = false;

static bool testLex(Tester& tester) {
	using namespace CMakeInterpreter;
	SourceRef begin { 0, 1, 1 };
	SourceRef end;
	std::vector<LexError> errors;
	auto input = readFile("LexInput.cmake");
	s_Lex = lexString(input, begin, end, errors);
	if (!errors.empty()) {
		s_LexSuccess = false;
		return false;
	}
	
	if (s_Lex.m_Root.m_Children.empty()) {
		s_LexSuccess = false;
		return false;
	}
	
	s_LexSuccess = true;
	return true;
}

static bool testNoArguments(Tester& tester) {
	using namespace CMakeInterpreter;
	
	if (!s_LexSuccess)
		return false;
	
	if (s_Lex.m_Root.m_Children.size() <= 0)
		return false;
	
	auto& no_arguments = s_Lex.m_Root.m_Children[0];
	if (no_arguments.m_Type != LexNodeType::CommandInvocation || no_arguments.m_Children.empty())
		return false;
	if (no_arguments.m_Children[0].m_Type != LexNodeType::Identifier || no_arguments.m_Children[0].m_Str != "no_arguments")
		return false;
	
	return true;
}

static bool testSpaceFirstNoArguments(Tester& tester) {
	using namespace CMakeInterpreter;
	
	if (!s_LexSuccess)
		return false;
	
	if (s_Lex.m_Root.m_Children.size() <= 1)
		return false;
	
	auto& space_first_no_arguments = s_Lex.m_Root.m_Children[1];
	if (space_first_no_arguments.m_Type != LexNodeType::CommandInvocation || space_first_no_arguments.m_Children.empty())
		return false;
	if (space_first_no_arguments.m_Children[0].m_Type != LexNodeType::Identifier || space_first_no_arguments.m_Children[0].m_Str != "space_first_no_arguments")
		return false;
	
	return true;
}

static bool testSpaceAfterNoArguments(Tester& tester) {
	using namespace CMakeInterpreter;
	
	if (!s_LexSuccess)
		return false;
	
	if (s_Lex.m_Root.m_Children.size() <= 2)
		return false;
	
	auto& space_after_no_arguments = s_Lex.m_Root.m_Children[2];
	if (space_after_no_arguments.m_Type != LexNodeType::CommandInvocation || space_after_no_arguments.m_Children.empty())
		return false;
	if (space_after_no_arguments.m_Children[0].m_Type != LexNodeType::Identifier || space_after_no_arguments.m_Children[0].m_Str != "space_after_no_arguments")
		return false;
	
	return true;
}

static bool testBracketArgument(Tester& tester) {
	using namespace CMakeInterpreter;
	
	if (!s_LexSuccess)
		return false;
	
	if (s_Lex.m_Root.m_Children.size() <= 3)
		return false;
	
	auto& bracket_argument = s_Lex.m_Root.m_Children[3];
	if (bracket_argument.m_Type != LexNodeType::CommandInvocation || bracket_argument.m_Children.size() != 2)
		return false;
	if (bracket_argument.m_Children[0].m_Type != LexNodeType::Identifier || bracket_argument.m_Children[0].m_Str != "bracket_argument")
		return false;
	auto& bracket_argument_args = bracket_argument.m_Children[1];
	if (bracket_argument_args.m_Type != LexNodeType::Arguments || bracket_argument_args.m_Children.empty() || bracket_argument_args.m_Children[0].m_Type != LexNodeType::BracketContent || bracket_argument_args.m_Children[0].m_Str != "arg1\n${VAR}")
		return false;
	
	return true;
}

static bool testQuotedArgument(Tester& tester) {
	using namespace CMakeInterpreter;
	
	if (!s_LexSuccess)
		return false;
	
	if (s_Lex.m_Root.m_Children.size() <= 4)
		return false;
	
	auto& quoted_argument = s_Lex.m_Root.m_Children[4];
	if (quoted_argument.m_Type != LexNodeType::CommandInvocation || quoted_argument.m_Children.size() != 2)
		return false;
	if (quoted_argument.m_Children[0].m_Type != LexNodeType::Identifier || quoted_argument.m_Children[0].m_Str != "quoted_argument")
		return false;
	auto& quoted_argument_args = quoted_argument.m_Children[1];
	if (quoted_argument_args.m_Type != LexNodeType::Arguments || quoted_argument_args.m_Children.empty() || quoted_argument_args.m_Children[0].m_Type != LexNodeType::QuotedArgument || quoted_argument_args.m_Children[0].m_Children.empty() || quoted_argument_args.m_Children[0].m_Children[0].m_Type != LexNodeType::QuotedElement || quoted_argument_args.m_Children[0].m_Children[0].m_Str != "arg1 ${VAR}")
		return false;
	
	return true;
}

static bool testUnquotedArgument(Tester& tester) {
	using namespace CMakeInterpreter;
	
	if (!s_LexSuccess)
		return false;
	
	if (s_Lex.m_Root.m_Children.size() <= 5)
		return false;
	
	auto& unquoted_argument = s_Lex.m_Root.m_Children[5];
	if (unquoted_argument.m_Type != LexNodeType::CommandInvocation || unquoted_argument.m_Children.size() != 2)
		return false;
	if (unquoted_argument.m_Children[0].m_Type != LexNodeType::Identifier || unquoted_argument.m_Children[0].m_Str != "unquoted_argument")
		return false;
	auto& unquoted_argument_args = unquoted_argument.m_Children[1];
	if (unquoted_argument_args.m_Type != LexNodeType::Arguments || unquoted_argument_args.m_Children.empty() || unquoted_argument_args.m_Children[0].m_Type != LexNodeType::UnquotedArgument || unquoted_argument_args.m_Children[0].m_Children.empty() || unquoted_argument_args.m_Children[0].m_Children[0].m_Type != LexNodeType::UnquotedElement || unquoted_argument_args.m_Children[0].m_Children[0].m_Str != "arg1${VAR}")
		return false;
	
	return true;
}
	
static bool testUnquotedLegacyArgument(Tester& tester) {
	using namespace CMakeInterpreter;
	
	if (!s_LexSuccess)
		return false;
	
	if (s_Lex.m_Root.m_Children.size() <= 6)
		return false;
	
	auto& unquoted_legacy_argument = s_Lex.m_Root.m_Children[6];
	if (unquoted_legacy_argument.m_Type != LexNodeType::CommandInvocation || unquoted_legacy_argument.m_Children.size() != 2)
		return false;
	if (unquoted_legacy_argument.m_Children[0].m_Type != LexNodeType::Identifier || unquoted_legacy_argument.m_Children[0].m_Str != "unquoted_legacy_argument")
		return false;
	auto& unquoted_legacy_argument_args = unquoted_legacy_argument.m_Children[1];
	if (unquoted_legacy_argument_args.m_Type != LexNodeType::Arguments || unquoted_legacy_argument_args.m_Children.empty() || unquoted_legacy_argument_args.m_Children[0].m_Type != LexNodeType::UnquotedLegacy || unquoted_legacy_argument_args.m_Children[0].m_Str != "arg=\"1\"")
		return false;
	
	return true;
}

static bool testUnquotedLegacyArgument2(Tester& tester) {
	using namespace CMakeInterpreter;
	
	if (!s_LexSuccess)
		return false;
	
	if (s_Lex.m_Root.m_Children.size() <= 7)
		return false;
	
	auto& unquoted_legacy_argument2 = s_Lex.m_Root.m_Children[7];
	if (unquoted_legacy_argument2.m_Type != LexNodeType::CommandInvocation || unquoted_legacy_argument2.m_Children.size() != 2)
		return false;
	if (unquoted_legacy_argument2.m_Children[0].m_Type != LexNodeType::Identifier || unquoted_legacy_argument2.m_Children[0].m_Str != "unquoted_legacy_argument2")
		return false;
	auto& unquoted_legacy_argument2_args = unquoted_legacy_argument2.m_Children[1];
	if (unquoted_legacy_argument2_args.m_Type != LexNodeType::Arguments || unquoted_legacy_argument2_args.m_Children.empty() || unquoted_legacy_argument2_args.m_Children[0].m_Type != LexNodeType::UnquotedLegacy || unquoted_legacy_argument2_args.m_Children[0].m_Str != "arg=$(1)")
		return false;
	
	return true;
}

static bool testSpaceBeforeArgument(Tester& tester) {
	using namespace CMakeInterpreter;
	
	if (!s_LexSuccess)
		return false;
	
	if (s_Lex.m_Root.m_Children.size() <= 8)
		return false;
	
	auto& space_before_argument = s_Lex.m_Root.m_Children[8];
	if (space_before_argument.m_Type != LexNodeType::CommandInvocation || space_before_argument.m_Children.size() != 2)
		return false;
	if (space_before_argument.m_Children[0].m_Type != LexNodeType::Identifier || space_before_argument.m_Children[0].m_Str != "space_before_argument")
		return false;
	auto& space_before_argument_args = space_before_argument.m_Children[1];
	if (space_before_argument_args.m_Type != LexNodeType::Arguments || space_before_argument_args.m_Children.empty() || space_before_argument_args.m_Children[0].m_Type != LexNodeType::UnquotedArgument || space_before_argument_args.m_Children[0].m_Children.empty() || space_before_argument_args.m_Children[0].m_Children[0].m_Type != LexNodeType::UnquotedElement || space_before_argument_args.m_Children[0].m_Children[0].m_Str != "arg1")
		return false;
	
	return true;
}

static bool testSpaceAfterArgument(Tester& tester) {
	using namespace CMakeInterpreter;
	
	if (!s_LexSuccess)
		return false;
	
	if (s_Lex.m_Root.m_Children.size() <= 9)
		return false;
	
	auto& space_after_argument = s_Lex.m_Root.m_Children[9];
	if (space_after_argument.m_Type != LexNodeType::CommandInvocation || space_after_argument.m_Children.size() != 2)
		return false;
	if (space_after_argument.m_Children[0].m_Type != LexNodeType::Identifier || space_after_argument.m_Children[0].m_Str != "space_after_argument")
		return false;
	auto& space_after_argument_args = space_after_argument.m_Children[1];
	if (space_after_argument_args.m_Type != LexNodeType::Arguments || space_after_argument_args.m_Children.empty() || space_after_argument_args.m_Children[0].m_Type != LexNodeType::UnquotedArgument || space_after_argument_args.m_Children[0].m_Children.empty() || space_after_argument_args.m_Children[0].m_Children[0].m_Type != LexNodeType::UnquotedElement || space_after_argument_args.m_Children[0].m_Children[0].m_Str != "arg1")
		return false;
	
	return true;
}

static bool testMultiArgument(Tester& tester) {
	using namespace CMakeInterpreter;
	
	if (!s_LexSuccess)
		return false;
	
	if (s_Lex.m_Root.m_Children.size() <= 10)
		return false;
	
	auto& multi_argument = s_Lex.m_Root.m_Children[10];
	if (multi_argument.m_Type != LexNodeType::CommandInvocation || multi_argument.m_Children.size() != 2)
		return false;
	if (multi_argument.m_Children[0].m_Type != LexNodeType::Identifier || multi_argument.m_Children[0].m_Str != "multi_argument")
		return false;
	auto& multi_argument_args = multi_argument.m_Children[1];
	if (multi_argument_args.m_Type != LexNodeType::Arguments || multi_argument_args.m_Children.size() != 3)
		return false;
	auto& bracketArg = multi_argument_args.m_Children[0];
	if (bracketArg.m_Type != LexNodeType::BracketContent || bracketArg.m_Str != "arg1")
		return false;
	auto& quotedArg = multi_argument_args.m_Children[1];
	if (quotedArg.m_Type != LexNodeType::QuotedArgument || quotedArg.m_Children.empty() || quotedArg.m_Children[0].m_Type != LexNodeType::QuotedElement || quotedArg.m_Children[0].m_Str != "arg2")
		return false;
	auto& unquotedArg = multi_argument_args.m_Children[2];
	if (unquotedArg.m_Type != LexNodeType::UnquotedArgument || unquotedArg.m_Children.empty() || unquotedArg.m_Children[0].m_Type != LexNodeType::UnquotedElement || unquotedArg.m_Children[0].m_Str != "arg3")
		return false;
	
	return true;
}

static bool testMultilineCommand(Tester& tester) {
	using namespace CMakeInterpreter;
	
	if (!s_LexSuccess)
		return false;
	
	if (s_Lex.m_Root.m_Children.size() <= 11)
		return false;
	
	auto& multiline_command = s_Lex.m_Root.m_Children[11];
	if (multiline_command.m_Type != LexNodeType::CommandInvocation || multiline_command.m_Children.size() != 2)
		return false;
	if (multiline_command.m_Children[0].m_Type != LexNodeType::Identifier || multiline_command.m_Children[0].m_Str != "multiline_command")
		return false;
	auto& multiline_command_args = multiline_command.m_Children[1];
	if (multiline_command_args.m_Type != LexNodeType::Arguments || multiline_command_args.m_Children.size() != 2)
		return false;
	auto& arg1 = multiline_command_args.m_Children[0];
	if (arg1.m_Type != LexNodeType::UnquotedArgument || arg1.m_Children.empty() || arg1.m_Children[0].m_Type != LexNodeType::UnquotedElement || arg1.m_Children[0].m_Str != "arg1")
		return false;
	auto& arg2 = multiline_command_args.m_Children[1];
	if (arg2.m_Type != LexNodeType::UnquotedArgument || arg2.m_Children.empty() || arg2.m_Children[0].m_Type != LexNodeType::UnquotedElement || arg2.m_Children[0].m_Str != "arg2")
		return false;
	
	return true;
}

static bool testArgumentsInArguments(Tester& tester) {
	using namespace CMakeInterpreter;
	
	if (!s_LexSuccess)
		return false;
	
	if (s_Lex.m_Root.m_Children.size() <= 12)
		return false;
	
	auto& arguments_in_arguments = s_Lex.m_Root.m_Children[12];
	if (arguments_in_arguments.m_Type != LexNodeType::CommandInvocation || arguments_in_arguments.m_Children.size() != 2)
		return false;
	if (arguments_in_arguments.m_Children[0].m_Type != LexNodeType::Identifier || arguments_in_arguments.m_Children[0].m_Str != "arguments_in_arguments")
		return false;
	auto& arguments_in_arguments_args = arguments_in_arguments.m_Children[1];
	if (arguments_in_arguments_args.m_Type != LexNodeType::Arguments || arguments_in_arguments_args.m_Children.empty())
		return false;
	auto& arguments = arguments_in_arguments_args.m_Children[0];
	if (arguments.m_Type != LexNodeType::Arguments || arguments.m_Children.size() != 3)
		return false;
	auto& bracketArg = arguments.m_Children[0];
	if (bracketArg.m_Type != LexNodeType::BracketContent || bracketArg.m_Str != "arg1")
		return false;
	auto& quotedArg = arguments.m_Children[1];
	if (quotedArg.m_Type != LexNodeType::QuotedArgument || quotedArg.m_Children.empty() || quotedArg.m_Children[0].m_Type != LexNodeType::QuotedElement || quotedArg.m_Children[0].m_Str != "arg2")
		return false;
	auto& unquotedArg = arguments.m_Children[2];
	if (unquotedArg.m_Type != LexNodeType::UnquotedArgument || unquotedArg.m_Children.empty() || unquotedArg.m_Children[0].m_Type != LexNodeType::UnquotedElement || unquotedArg.m_Children[0].m_Str != "arg3")
		return false;
	
	return true;
}

static bool testEscapeCharacters(Tester& tester) {
	using namespace CMakeInterpreter;
	
	if (!s_LexSuccess)
		return false;
	
	if (s_Lex.m_Root.m_Children.size() <= 13)
		return false;
	
	auto& escape_characters = s_Lex.m_Root.m_Children[13];
	if (escape_characters.m_Type != LexNodeType::CommandInvocation || escape_characters.m_Children.size() != 2)
		return false;
	if (escape_characters.m_Children[0].m_Type != LexNodeType::Identifier || escape_characters.m_Children[0].m_Str != "escape_characters")
		return false;
	auto& escape_characters_args = escape_characters.m_Children[1];
	if (escape_characters_args.m_Type != LexNodeType::Arguments || escape_characters_args.m_Children.size() != 2)
		return false;
	auto& arg1 = escape_characters_args.m_Children[0];
	if (arg1.m_Type != LexNodeType::QuotedArgument || arg1.m_Children.size() != 4 || arg1.m_Children[0].m_Type != LexNodeType::EscapeEncoded || arg1.m_Children[0].m_Str != "r" || arg1.m_Children[1].m_Type != LexNodeType::EscapeEncoded || arg1.m_Children[1].m_Str != "t" || arg1.m_Children[2].m_Type != LexNodeType::EscapeEncoded || arg1.m_Children[2].m_Str != "n" || arg1.m_Children[3].m_Type != LexNodeType::EscapeSemicolon)
		return false;
	auto& arg2 = escape_characters_args.m_Children[1];
	if (arg2.m_Type != LexNodeType::UnquotedArgument || arg2.m_Children.empty() || arg2.m_Children[0].m_Type != LexNodeType::EscapeIdentity || arg2.m_Children[0].m_Str != ")")
		return false;
	
	return true;
}

struct LexerTestsRegister {
	LexerTestsRegister() {
		auto& tester = Tester::Get();
		tester.addTest("CMakeLexer", "Lex", &testLex);
		tester.addTest("CMakeLexer", "no_arguments", &testNoArguments);
		tester.addTest("CMakeLexer", "space_first_no_arguments", &testSpaceFirstNoArguments);
		tester.addTest("CMakeLexer", "space_after_no_arguments", &testSpaceAfterNoArguments);
		tester.addTest("CMakeLexer", "bracket_argument", &testBracketArgument);
		tester.addTest("CMakeLexer", "quoted_argument", &testQuotedArgument);
		tester.addTest("CMakeLexer", "unquoted_argument", &testUnquotedArgument);
		tester.addTest("CMakeLexer", "unquoted_legacy_argument", &testUnquotedLegacyArgument);
		tester.addTest("CMakeLexer", "unquoted_legacy_argument2", &testUnquotedLegacyArgument2);
		tester.addTest("CMakeLexer", "space_before_argument", &testSpaceBeforeArgument);
		tester.addTest("CMakeLexer", "space_after_argument", &testSpaceAfterArgument);
		tester.addTest("CMakeLexer", "multi_argument", &testMultiArgument);
		tester.addTest("CMakeLexer", "multiline_command", &testMultilineCommand);
		tester.addTest("CMakeLexer", "arguments_in_arguments", &testArgumentsInArguments);
		tester.addTest("CMakeLexer", "escape_characters", &testEscapeCharacters);
	}
} lexerTestsRegister;
