#include "CMakeCompiler/Interpreter.h"
#include "CMakeCompiler/Lexer.h"

#include <iostream>

namespace CMakeCompiler {
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
			runtimeError("Expected FileElement", fileElement);
			return;
		}

		auto& fileElementChildren = fileElement.m_Children;
		if (fileElementChildren.empty()) {
			runtimeError("Expected CommandInvocation", fileElement);
			return;
		}

		auto& commandInvocation = fileElementChildren[0];
		if (commandInvocation.m_Type != LexNodeType::CommandInvocation) {
			runtimeError("Expected CommandInvocation", commandInvocation);
			return;
		}

		auto& commandInvocationChildren = commandInvocation.m_Children;
		if (commandInvocationChildren.size() < 2) {
			runtimeError("Expected Identifier and Arguments", commandInvocation);
			return;
		}

		auto& identifier = commandInvocationChildren[0];
		if (identifier.m_Type != LexNodeType::Identifier) {
			runtimeError("Expected Identifier", identifier);
			return;
		}

		auto& arguments = commandInvocationChildren[1];
		if (arguments.m_Type != LexNodeType::Arguments) {
			runtimeError("Expected Arguments", arguments);
			return;
		}

		auto function = getFunction(std::string { identifier.m_Str });
		if (!function) {
			runtimeError("Function not found", identifier);
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
						runtimeError("Unexpected element type", element);
						return;
					}
				}

				evalVariableRefs(str);
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
						runtimeError("Unexpected element type", element);
						return;
					}
				}

				evalVariableRefs(str, true);
				args += str;
				break;
			}
			case LexNodeType::UnquotedLegacy:
				runtimeError("UnquotedLegacy isn't supported yet", argument);
				break;
			default:
				runtimeError("Unexpected argument type", argument);
				return;
			}
		}

		function(*this, args);
		++m_CurrentCommand;
	}

	void InterpreterState::evalVariableRefs(std::string& str, bool allowLegacy) {
		for (std::size_t i = 0; i < str.size(); ++i) {
			if (str[i] == '$') {
				if (++i >= str.size())
					break;
				if (str[i] == '{') {
					std::size_t len = 1;
					while (i + len < str.size() && str[i + len] != '}')
						++len;
					if (str[i + len] == '}') {
						std::string variableName = str.substr(i + 1, len - 1);

						auto value = getVariable(variableName);
						if (value.empty()) {
							str.erase(i - 1, len + 2);
							i -= 2;
						} else {
							str.replace(i - 1, len + 2, value);
							i += value.size();
						}
					}
				} else if (allowLegacy && str[i] == '(') {
					std::size_t len = 1;
					while (i + len < str.size() && str[i + len] != ')')
						++len;
					if (str[i + len] == ')') {
						std::string variableName = str.substr(i + 1, len - 1);

						auto value = getVariable(variableName);
						if (value.empty()) {
							str.erase(i - 1, len + 2);
							i -= 2;
						} else {
							str.replace(i - 1, len + 2, value);
							i += value.size();
						}
					}
				}
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

	static void printSourceRef(CMakeCompiler::SourceRef sourceRef) {
		std::cout << sourceRef.m_Index << ": " << sourceRef.m_Line << ", " << sourceRef.m_Column;
	}

	void InterpreterState::runtimeError(std::string_view message) {
		if (m_CurrentCommand < m_Lex->m_Root.m_Children.size()) {
			runtimeError(message, m_Lex->m_Root.m_Children[m_CurrentCommand]);
		} else {
			runtimeError(message, {});
		}
	}

	void InterpreterState::runtimeError(std::string_view message, const LexNode& node) {
		std::cerr << message << ", at (";
		printSourceRef(node.m_Begin);
		std::cerr << ")\n";
		m_Borked = true;
	}
} // namespace CMakeCompiler