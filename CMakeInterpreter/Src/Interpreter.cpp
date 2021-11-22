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

		if (offset < args.size())
			splittedArgs.push_back(std::string { args.substr(offset) });
		return splittedArgs;
	}

	InterpreterState::InterpreterState(Lex* lex)
	    : m_Lex(lex) { }

	bool InterpreterState::hasNext() {
		return !m_Borked && m_CurrentCommand < m_Lex->m_Root.m_Children.size();
	}

	void InterpreterState::next() {
		if (!hasNext())
			return;

		auto& fileElement = m_Lex->m_Root.m_Children[m_CurrentCommand];
		if (fileElement.m_Type != LexNodeType::FileElement) {
			++m_CurrentCommand;
			return;
		}

		auto& fileElementChildren = fileElement.m_Children;
		if (fileElementChildren.empty()) {
			++m_CurrentCommand;
			return;
		}

		auto& commandInvocation = fileElementChildren[0];
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

	FunctionCallback InterpreterState::getFunction(const std::string& function) {
		auto itr = m_Functions.find(function);
		if (itr != m_Functions.end())
			return itr->second;
		return nullptr;
	}

	std::string_view InterpreterState::getVariable(const std::string& variable) {
		auto itr = m_Variables.find(variable);
		if (itr != m_Variables.end())
			return itr->second;
		return {};
	}

	std::string_view InterpreterState::getCachedVariable(const std::string& variable) {
		auto itr = m_CachedVariables.find(variable);
		if (itr != m_CachedVariables.end())
			return itr->second;
		return {};
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
