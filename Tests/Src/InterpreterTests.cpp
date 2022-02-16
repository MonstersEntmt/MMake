#include "FileIO.h"
#include "Test.h"

#include <CMakeInterpreter/Interpreter.h>
#include <CMakeInterpreter/Lexer.h>

#include <iostream>

static CMakeInterpreter::Lex              s_Lex;
static CMakeInterpreter::InterpreterState s_State { nullptr };
[[maybe_unused]] static bool              s_LexSuccess = false;

static void testPrintVars(CMakeInterpreter::InterpreterState& state, [[maybe_unused]] std::string_view args)
{
	std::unordered_map<std::string, std::string> allAvailableVariables;
	for (auto itr = state.m_Variables.rbegin(); itr != state.m_Variables.rend(); ++itr)
	{
		auto& variables = *itr;
		for (auto& variable : variables)
			allAvailableVariables.insert({ variable.first, variable.second });
	}

	for (auto& var : allAvailableVariables)
		std::cout << "\"" << var.first << "\" = \"" << var.second << "\"\n";
}

static void testClearVars(CMakeInterpreter::InterpreterState& state, [[maybe_unused]] std::string_view args)
{
	for (auto itr = state.m_Variables.rbegin(); itr != state.m_Variables.rend(); ++itr)
	{
		itr->clear();
	}
}

static bool testLex([[maybe_unused]] Tester& tester)
{
	using namespace CMakeInterpreter;
	SourceRef             begin { 0, 1, 1 };
	SourceRef             end;
	std::vector<LexError> errors;
	auto                  input = readFile("InterpretInput.cmake");
	s_Lex                       = lexString(input, begin, end, errors);
	if (!errors.empty())
	{
		s_LexSuccess = false;
		return false;
	}

	if (s_Lex.m_Root.m_Children.size() != 31)
	{
		s_LexSuccess = false;
		return false;
	}

	s_State = { &s_Lex };
	s_State.addDefaultFunctions();
	s_State.addFunction("printVars", &testPrintVars);
	s_State.addFunction("clearVars", &testClearVars);

	s_LexSuccess = true;
	return true;
}

static bool testNoArguments([[maybe_unused]] Tester& tester)
{
	for (std::size_t i = 0; i < 2; ++i)
	{
		if (!s_State.hasNext())
			return false;

		s_State.next();
	}
	return true;
}

static bool testOneArgument([[maybe_unused]] Tester& tester)
{
	for (std::size_t i = 0; i < 2; ++i)
	{
		if (!s_State.hasNext())
			return false;

		s_State.next();
	}
	return true;
}

static bool testTwoArguments([[maybe_unused]] Tester& tester)
{
	for (std::size_t i = 0; i < 2; ++i)
	{
		if (!s_State.hasNext())
			return false;

		s_State.next();
	}
	return true;
}

static bool testVar([[maybe_unused]] Tester& tester)
{
	for (std::size_t i = 0; i < 3; ++i)
	{
		if (!s_State.hasNext())
			return false;

		s_State.next();
	}
	return true;
}

static bool testFunctionScope([[maybe_unused]] Tester& tester)
{
	for (std::size_t i = 0; i < 3; ++i)
	{
		if (!s_State.hasNext())
			return false;

		s_State.next();
	}
	return true;
}

static bool testMacroScope([[maybe_unused]] Tester& tester)
{
	for (std::size_t i = 0; i < 3; ++i)
	{
		if (!s_State.hasNext())
			return false;

		s_State.next();
	}
	return true;
}

static bool testVarReplacements([[maybe_unused]] Tester& tester)
{
	for (std::size_t i = 0; i < 4; ++i)
	{
		if (!s_State.hasNext())
			return false;

		s_State.next();
	}
	return true;
}

struct InterpreterTestsRegister
{
	InterpreterTestsRegister()
	{
		auto& tester = Tester::Get();
		tester.addTest("CMakeInterpreter", "Lex", &testLex);
		tester.addTest("CMakeInterpreter", "NoArguments", &testNoArguments);
		tester.addTest("CMakeInterpreter", "OneArgument", &testOneArgument);
		tester.addTest("CMakeInterpreter", "TwoArguments", &testTwoArguments);
		tester.addTest("CMakeInterpreter", "Var", &testVar);
		tester.addTest("CMakeInterpreter", "FunctionScope", &testFunctionScope);
		tester.addTest("CMakeInterpreter", "MacroScope", &testMacroScope);
		tester.addTest("CMakeInterpreter", "VarReplacements", &testVarReplacements);
	}
};
// [[maybe_unused]] InterpreterTestsRegister interpreterTestsRegister;