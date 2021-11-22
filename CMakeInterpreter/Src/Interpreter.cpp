#include "CMakeInterpreter/Interpreter.h"
#include "CMakeInterpreter/Lexer.h"

#include <iostream>
#include <sstream>

namespace CMakeInterpreter {
	std::vector<std::string> splitArguments(std::string_view args) {
		std::vector<std::string> splittedArgs;
		std::size_t offset = 0;
		bool escaped       = false;
		for (std::size_t i = 0; i < args.size(); ++i) {
			switch (args[i]) {
			case '\\':
				escaped = !escaped;
				break;
			case ';':
				if (!escaped) {
					splittedArgs.push_back(std::string { args.substr(offset, i - offset) });
					offset = i + 1;
				}
				[[fallthrough]];
			default:
				escaped = false;
				break;
			}
		}
		
		if (offset < args.size())
			splittedArgs.push_back(std::string { args.substr(offset) });

		for (auto& arg : splittedArgs) {
			for (std::size_t i = 0; i < arg.size(); ++i) {
				if (arg[i] == '\\') {
					if (++i >= arg.size())
						break;

					if (arg[i] == ';')
						arg.replace(i - 1, 2, 1, ';');
				}
			}
		}
		return splittedArgs;
	}
	
	void cmake_set(InterpreterState& state, std::string_view args) {
		auto variableNameEnd = args.find_first_of(';');
		if (variableNameEnd >= args.size() - 1) {
			state.runtimeError("set requires multiple arguments");
			return;
		}

		std::string_view variableName = args.substr(0, variableNameEnd);
		auto valueStart = variableNameEnd + 1;
		auto valueEnd = args.rfind("PARENT_SCOPE");
		
		state.addVariable(variableName, args.substr(valueStart, valueEnd - valueStart), valueEnd < args.size());
	}
	
	void cmake_unset(InterpreterState& state, std::string_view args) {
		auto variableNameEnd = args.find_first_of(';');
		state.removeVariable(args.substr(0, variableNameEnd));
	}
	
	void cmake_function_callback(std::size_t startLine, std::size_t endLine, const std::string& argNames, InterpreterState& state, std::string_view args) {
		auto entry = state.m_CurrentCommand;
		state.m_CurrentCommand = startLine;
		state.startScope();
		
		std::vector<std::string_view> splittedArgs;
		std::size_t offset = 0;
		bool escaped       = false;
		for (std::size_t i = 0; i < args.size(); ++i) {
			switch (args[i]) {
			case '\\':
				escaped = !escaped;
				break;
			case ';':
				if (!escaped) {
					splittedArgs.push_back(args.substr(offset, i - offset));
					offset = i + 1;
				}
				[[fallthrough]];
			default:
				escaped = false;
				break;
			}
		}
		
		if (offset < args.size())
			splittedArgs.push_back(args.substr(offset));
		
		std::vector<std::string_view> splittedArgNames;
		offset = 0;
		escaped       = false;
		for (std::size_t i = 0; i < argNames.size(); ++i) {
			switch (argNames[i]) {
			case '\\':
				escaped = !escaped;
				break;
			case ';':
				if (!escaped) {
					splittedArgNames.push_back(argNames.substr(offset, i - offset));
					offset = i + 1;
				}
				[[fallthrough]];
			default:
				escaped = false;
				break;
			}
		}
		
		if (offset < argNames.size())
			splittedArgNames.push_back(argNames.substr(offset));
		
		state.addVariable("ARGC", std::to_string(splittedArgs.size()));
		for (std::size_t i = 0; i < splittedArgs.size(); ++i) {
			state.addVariable("ARGV" + std::to_string(1 + i), splittedArgs[i]);
			if (i < splittedArgNames.size())
				state.addVariable(splittedArgNames[i], splittedArgs[i]);
		}
		
		while (state.m_CurrentCommand <= endLine && state.hasNext()) {
			state.next();
		}
		state.endScope(),
		state.m_CurrentCommand = entry;
	}
	
	void cmake_function(InterpreterState& state, std::string_view args) {
		auto functionNameEnd = args.find_first_of(';');
		std::string_view functionName = args.substr(0,  functionNameEnd);
		std::string_view argNames;
		if (functionNameEnd < args.size() - 1)
			argNames = args.substr(functionNameEnd + 1);
		std::size_t start = state.m_CurrentCommand + 1;
		std::size_t end = start;
		std::size_t layers = 1;
		for (; end < state.m_Lex->m_Root.m_Children.size(); ++end) {
			auto& commandInvocation = state.m_Lex->m_Root.m_Children[end];
			if (commandInvocation.m_Type != LexNodeType::CommandInvocation)
				continue;
			
			auto& commandInvocationChildren = commandInvocation.m_Children;
			if (commandInvocationChildren.size() < 2)
				continue;

			auto& identifier = commandInvocationChildren[0];
			if (identifier.m_Type != LexNodeType::Identifier)
				continue;
			
			if (identifier.m_Str == "function")
				++layers;
			
			if (identifier.m_Str == "endfunction")
				if (--layers == 0)
					break;
		}
		
		state.addFunction(functionName, cmake_function_callback, start, end - 1, std::string { argNames });
		state.m_CurrentCommand = end;
	}
	
	void cmake_endfunction(InterpreterState& state, std::string_view args) {}
	
	void cmake_message(InterpreterState& state, std::string_view args) {
		std::cout << args << std::endl;
	}
	
	InterpreterState::InterpreterState(Lex* lex)
	    : m_Lex(lex) {
		startScope();
	}

	void InterpreterState::addDefaultFunctions() {
		addFunction("set", &cmake_set);
		addFunction("unset", &cmake_unset);
		addFunction("function", &cmake_function);
		addFunction("endfunction", &cmake_endfunction);
		addFunction("message", &cmake_message);
	}
	
	void InterpreterState::addBoundFunction(std::string_view name, FunctionCallback callback) {
		if (m_Functions.empty())
			m_Functions.push_back({});
		
		auto& funcs = m_Functions[m_Functions.size() - 1];
		funcs.insert({ std::string{ name }, callback });
	}
	
	void InterpreterState::removeFunction(std::string_view name) {
		std::string nm = std::string{ name };
		for (auto itr = m_Functions.rbegin(); itr != m_Functions.rend(); ++itr) {
			auto& functions = *itr;
			auto func = functions.find(nm);
			if (func != functions.end()) {
				functions.erase(func);
				return;
			}
		}
	}
	
	void InterpreterState::addVariable(std::string_view name, std::string_view value, bool parent) {
		if (m_Variables.empty())
			m_Variables.push_back({});
		
		if (parent && m_Variables.size() < 2)
			return;
		
		auto& vars = m_Variables[m_Variables.size() - (parent ? 2 : 1)];
		vars.insert_or_assign(std::string{ name }, std::string{ value });
	}
	
	void InterpreterState::removeVariable(std::string_view name) {
		std::string nm = std::string{ name };
		for (auto itr = m_Variables.rbegin(); itr != m_Variables.rend(); ++itr) {
			auto& variables = *itr;
			auto var = variables.find(nm);
			if (var != variables.end()) {
				variables.erase(var);
				return;
			}
		}
	}
	
	void InterpreterState::addCachedVariable(std::string_view name, std::string_view value, bool overwrite) {
		if (overwrite)
			m_CachedVariables.insert_or_assign(std::string { name }, std::string { value });
		else
			m_CachedVariables.insert({ std::string { name }, std::string { value } });
	}

	void InterpreterState::removeCachedVariable(std::string_view name) {
		auto itr = m_CachedVariables.find(std::string { name });
		if (itr != m_CachedVariables.end())
			m_CachedVariables.erase(itr);
	}
	
	FunctionCallback InterpreterState::getFunction(const std::string& function) {
		for (auto itr = m_Functions.rbegin(); itr != m_Functions.rend(); ++itr) {
			auto& functions = *itr;
			auto func = functions.find(function);
			if (func != functions.end())
				return func->second;
		}
		return nullptr;
	}

	std::string_view InterpreterState::getVariable(const std::string& variable) {
		for (auto itr = m_Variables.rbegin(); itr != m_Variables.rend(); ++itr) {
			auto& variables = *itr;
			auto var = variables.find(variable);
			if (var != variables.end())
				return var->second;
		}
		return {};
	}

	std::string_view InterpreterState::getCachedVariable(const std::string& variable) {
		auto itr = m_CachedVariables.find(variable);
		if (itr != m_CachedVariables.end())
			return itr->second;
		return {};
	}
	
	void InterpreterState::startScope() {
		m_Variables.push_back({});
		m_Functions.push_back({});
	}
	
	void InterpreterState::endScope() {
		m_Variables.pop_back();
		m_Functions.pop_back();
	}

	bool InterpreterState::hasNext() {
		return !m_Borked && m_CurrentCommand < m_Lex->m_Root.m_Children.size();
	}

	void InterpreterState::next() {
		if (!hasNext())
			return;

		auto& commandInvocation = m_Lex->m_Root.m_Children[m_CurrentCommand];
		if (commandInvocation.m_Type != LexNodeType::CommandInvocation) {
			++m_CurrentCommand;
			return;
		}

		auto& commandInvocationChildren = commandInvocation.m_Children;
		if (commandInvocationChildren.size() < 2) {
			++m_CurrentCommand;
			return;
		}

		auto& identifier = commandInvocationChildren[0];
		if (identifier.m_Type != LexNodeType::Identifier) {
			++m_CurrentCommand;
			return;
		}

		auto& arguments = commandInvocationChildren[1];
		if (arguments.m_Type != LexNodeType::Arguments) {
			++m_CurrentCommand;
			return;
		}

		auto function = getFunction(std::string { identifier.m_Str });
		if (!function) {
			runtimeError("Function '" + std::string { identifier.m_Str } + "' not found", identifier.m_Begin);
			return;
		}

		std::string args;
		for (auto& argument : arguments.m_Children) {
			if (!args.empty())
				args += ';';

			switch (argument.m_Type) {
			case LexNodeType::BracketContent:
				args += argument.m_Str;
				break;
			case LexNodeType::QuotedArgument: {
				std::string str;
				for (auto& element : argument.m_Children) {
					switch (element.m_Type) {
					case LexNodeType::EscapeIdentity:
						str += element.m_Str;
						break;
					case LexNodeType::EscapeEncoded:
						if (element.m_Str == "t")
							str += '\t';
						else if (element.m_Str == "r")
							str += '\r';
						else if (element.m_Str == "n")
							str += '\n';
						break;
					case LexNodeType::EscapeSemicolon:
						str += "\\;";
						break;
					case LexNodeType::QuotedElement:
						str += element.m_Str;
						break;
					default:
						break;
					}
				}

				evalVariableRefs(str, argument.m_Begin);
				args += str;
				break;
			}
			case LexNodeType::UnquotedArgument: {
				std::string str;
				for (auto& element : argument.m_Children) {
					switch (element.m_Type) {
					case LexNodeType::EscapeIdentity:
						str += element.m_Str;
						break;
					case LexNodeType::EscapeEncoded:
						if (element.m_Str == "t")
							str += '\t';
						else if (element.m_Str == "r")
							str += '\r';
						else if (element.m_Str == "n")
							str += '\n';
						break;
					case LexNodeType::EscapeSemicolon:
						str += "\\;";
						break;
					case LexNodeType::UnquotedElement:
						str += element.m_Str;
						break;
					default:
						break;
					}
				}

				evalVariableRefs(str, argument.m_Begin);
				args += str;
				break;
			}
			case LexNodeType::UnquotedLegacy: {
				std::string str = std::string { argument.m_Str };
				evalVariableRefs(str, argument.m_Begin);
				args += str;
				break;
			}
			default:
				break;
			}
		}

		function(*this, args);
		++m_CurrentCommand;
	}

	void InterpreterState::evalVariableRefs(std::string& str, SourceRef at) {
		for (std::size_t i = str.size() - 1; i != ~0ULL; --i) {
			if (str[i] == '$') {
				std::size_t start = i;
				++i;

				std::size_t len = 0;
				while (i + len < str.size() && str[i + len] != '{')
					++len;
				std::string type = str.substr(i, len);
				i += len;
				if (i < str.size() && str[i] == '{') {
					len = 0;
					while (i + len < str.size() && str[i + len] != '}')
						++len;
					if (str[i + len] == '}') {
						std::string variableName = str.substr(i + 1, len - 1);
						i += len;

						std::string_view value;
						if (type == "ENV") {
							auto result = std::getenv(variableName.c_str());
							if (result)
								value = result;
						} else if (type == "CACHE") {
							value = getCachedVariable(variableName);
						} else {
							value = getVariable(variableName);
						}

						if (value.empty()) {
							str.erase(start, 1 + i - start);
							
							SourceRef varAt = at;
							varAt.m_Index += start;
							varAt.m_Column += start;
							if (type == "ENV") {
								runtimeWarning("Environment variable '" + variableName + "' isn't set!", varAt);
							} else if (type == "CACHE") {
								runtimeWarning("Cached variable '" + variableName + "' isn't set!", varAt);
							} else {
								runtimeWarning("Variable '" + variableName + "' isn't set!", varAt);
							}
						} else {
							str.replace(start, 1 + i - start, value);
						}
					}
				}

				i = start;
			}
		}
	}

	void InterpreterState::runtimeError(std::string_view message) {
		if (m_CurrentCommand < m_Lex->m_Root.m_Children.size()) {
			runtimeError(message, m_Lex->m_Root.m_Children[m_CurrentCommand].m_Begin);
		} else {
			runtimeError(message, {});
		}
	}

	void InterpreterState::runtimeError(std::string_view message, SourceRef at) {
		std::ostringstream str;
		str << "CMake Runtime Error: " << message << "\n";
		auto lineBegin = m_Lex->m_Source.find_last_of('\n', at.m_Index);
		if (lineBegin >= m_Lex->m_Source.size())
			lineBegin = 0;
		auto lineEnd             = m_Lex->m_Source.find_first_of('\n', at.m_Index);
		std::string_view strview = m_Lex->m_Source;
		std::string lineNr       = std::to_string(at.m_Line) + ": ";
		str << lineNr << strview.substr(lineBegin + 1, lineEnd - lineBegin - 1);
		str << '\n'
			<< std::string(lineNr.size() + at.m_Column - 1, ' ') << "^\n";
		std::cerr << str.str();
		m_Borked = true;
	}
	
	void InterpreterState::runtimeWarning(std::string_view message) {
		if (m_CurrentCommand < m_Lex->m_Root.m_Children.size()) {
			runtimeWarning(message, m_Lex->m_Root.m_Children[m_CurrentCommand].m_Begin);
		} else {
			runtimeWarning(message, {});
		}
	}

	void InterpreterState::runtimeWarning(std::string_view message, SourceRef at) {
		std::ostringstream str;
		str << "CMake Runtime Warning: " << message << "\n";
		auto lineBegin = m_Lex->m_Source.find_last_of('\n', at.m_Index);
		if (lineBegin >= m_Lex->m_Source.size())
			lineBegin = 0;
		auto lineEnd             = m_Lex->m_Source.find_first_of('\n', at.m_Index);
		std::string_view strview = m_Lex->m_Source;
		std::string lineNr       = std::to_string(at.m_Line) + ": ";
		str << lineNr << strview.substr(lineBegin + 1, lineEnd - lineBegin - 1);
		str << '\n'
			<< std::string(lineNr.size() + at.m_Column - 1, ' ') << "^\n";
		std::cout << str.str();
	}
} // namespace CMakeInterpreter
