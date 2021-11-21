#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace CMakeInterpreter {
	struct Lex;
	struct LexNode;

	struct InterpreterState;

	using FunctionCallback = std::function<void(InterpreterState& interpreter, std::string_view args)>;

	std::vector<std::string> splitArguments(std::string_view args);

	struct InterpreterState {
	public:
		InterpreterState(Lex* lex);

		bool hasNext();
		void next();

		void evalVariableRefs(std::string& str);

		FunctionCallback getFunction(const std::string& function);
		std::string_view getVariable(const std::string& variable);
		std::string_view getCachedVariable(const std::string& variable);

		void runtimeError(std::string_view message);
		void runtimeError(std::string_view message, const LexNode& node);

	public:
		std::unordered_map<std::string, FunctionCallback> m_Functions;
		std::unordered_map<std::string, std::string> m_Variables;
		std::unordered_map<std::string, std::string> m_CachedVariables;
		std::size_t m_CurrentCommand = 0;
		Lex* m_Lex;

	private:
		bool m_Borked = false;
	};
} // namespace CMakeInterpreter