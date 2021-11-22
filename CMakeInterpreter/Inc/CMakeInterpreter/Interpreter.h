#pragma once

#include "Lexer.h"

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace CMakeInterpreter {
	struct InterpreterState;

	using FunctionCallback = std::function<void(InterpreterState& interpreter, std::string_view args)>;

	std::vector<std::string> splitArguments(std::string_view args);
	
	struct InterpreterState {
	public:
		InterpreterState(Lex* lex);

		void addDefaultFunctions();
		
		template <class Func>
		void addFunction(std::string_view name, Func&& callback) {
			addBoundFunction(name, std::bind(std::move(callback), std::placeholders::_1, std::placeholders::_2));
		}
		
		template <class Func, class T>
		void addFunction(std::string_view name, Func&& callback, T* obj) {
			addBoundFunction(name, std::bind(std::move(callback), obj, std::placeholders::_1, std::placeholders::_2));
		}
		
		template <class Func, class... Args>
		void addFunction(std::string_view name, Func&& callback, Args&&... args) {
			addBoundFunction(name, std::bind(std::move(callback), std::forward<Args>(args)..., std::placeholders::_1, std::placeholders::_2));
		}
		
		void addBoundFunction(std::string_view name, FunctionCallback callback);
		void removeFunction(std::string_view name);
		void addVariable(std::string_view name, std::string_view value, bool parent = false);
		void removeVariable(std::string_view name);
		void addCachedVariable(std::string_view name, std::string_view value, bool overwrite = false);
		void removeCachedVariable(std::string_view name);
		
		FunctionCallback getFunction(const std::string& function);
		std::string_view getVariable(const std::string& variable);
		std::string_view getCachedVariable(const std::string& variable);
		
		void startScope();
		void endScope();
		
		bool hasNext();
		void next();

		void evalVariableRefs(std::string& str, SourceRef at);

		void runtimeError(std::string_view message);
		void runtimeError(std::string_view message, SourceRef at);
		void runtimeWarning(std::string_view message);
		void runtimeWarning(std::string_view message, SourceRef at);

	public:
		std::vector<std::unordered_map<std::string, FunctionCallback>> m_Functions;
		std::vector<std::unordered_map<std::string, std::string>> m_Variables;
		std::unordered_map<std::string, std::string> m_CachedVariables;
		std::size_t m_CurrentCommand = 0;
		Lex* m_Lex;

	private:
		bool m_Borked = false;
	};
} // namespace CMakeInterpreter
