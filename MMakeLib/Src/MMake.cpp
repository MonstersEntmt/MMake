#include "MMake/MMake.h"

#include <CommonCLI/Colors.h>
#include <Premake/Defines.h>

#ifdef MMAKE_CMAKE_INTERPRETER
	#include <CMakeInterpreter/Lexer.h>
	#include <CMakeInterpreter/Interpreter.h>
#endif

#if PREMAKE_IS_CONFIG_DEBUG
	#define PICCOLO_ENABLE_DEBUG_LIB
#endif

extern "C" {
#include <Piccolo/debug/disassembler.h>
#include <Piccolo/include.h>
#include <Piccolo/stdlib/stdlib.h>
}

#include <cstdarg>

#include <filesystem>
#include <iostream>
#include <sstream>

namespace MMake {
	static void printPiccoloError(const char* format, std::va_list args) {
		std::ostringstream error;
		error << CommonCLI::Colors::Error;
		int size      = std::vsnprintf(nullptr, 0, format, args);
		char* buf     = new char[size + 2];
		buf[size + 1] = '\0';
		std::vsnprintf(buf, size + 1, format, args);
		error << buf;
		delete[] buf;
		std::cout << error.str();
	}

	void run() {
		piccolo_Engine* engine = new piccolo_Engine();
		piccolo_initEngine(engine, &printPiccoloError);
		piccolo_addIOLib(engine);
		piccolo_addTimeLib(engine);
#if PREMAKE_IS_CONFIG_DEBUG
		piccolo_addDebugLib(engine);
#endif

		std::filesystem::path mainPicFile = std::filesystem::current_path() / "mmake.pic";
		piccolo_Package* package          = piccolo_loadPackage(engine, mainPicFile.string().c_str());
		if (package->compilationError) {
			piccolo_freeEngine(engine);
			delete engine;
			return;
		}

		if (!piccolo_executePackage(engine, package)) {
			piccolo_enginePrintError(engine, "Runtime error.\n");
		}

		piccolo_freeEngine(engine);
	}

#ifdef MMAKE_CMAKE_INTERPRETER
	static void printLexNodeType(CMakeInterpreter::LexNodeType type) {
		using namespace CMakeInterpreter;
		switch (type) {
		case LexNodeType::File:
			std::cout << "File";
			break;
		case LexNodeType::CommandInvocation:
			std::cout << "CommandInvocation";
			break;
		case LexNodeType::Identifier:
			std::cout << "Identifier";
			break;
		case LexNodeType::Arguments:
			std::cout << "Arguments";
			break;
		case LexNodeType::BracketContent:
			std::cout << "BracketContent";
			break;
		case LexNodeType::QuotedArgument:
			std::cout << "QuotedArgument";
			break;
		case LexNodeType::QuotedElement:
			std::cout << "QuotedElement";
			break;
		case LexNodeType::UnquotedArgument:
			std::cout << "UnquotedArgument";
			break;
		case LexNodeType::UnquotedElement:
			std::cout << "UnquotedElement";
			break;
		case LexNodeType::UnquotedLegacy:
			std::cout << "UnquotedLegacy";
			break;
		case LexNodeType::EscapeIdentity:
			std::cout << "EscapeIdentity";
			break;
		case LexNodeType::EscapeEncoded:
			std::cout << "EscapeEncoded";
			break;
		case LexNodeType::EscapeSemicolon:
			std::cout << "EscapeSemicolon";
			break;
		default:
			std::cout << "Unknown";
			break;
		}
	}

	static void printSourceRef(CMakeInterpreter::SourceRef sourceRef) {
		std::cout << sourceRef.m_Index << ": " << sourceRef.m_Line << ", " << sourceRef.m_Column;
	}

	static void printLexNode(const CMakeInterpreter::LexNode& node, std::size_t tabs = 0) {
		std::cout << std::string(tabs, ' ');
		printLexNodeType(node.m_Type);
		std::cout << ": '" << node.m_Str << "' (";
		printSourceRef(node.m_Begin);
		std::cout << " <-> ";
		printSourceRef(node.m_End);
		std::cout << ")\n";
		for (auto& child : node.m_Children)
			printLexNode(child, tabs + 1);
	}

	static void printLex(const CMakeInterpreter::Lex& lex) {
		std::cout << "Lex:\n";
		printLexNode(lex.m_Root, 1);
	}

	void runCMake() {
		using namespace CMakeInterpreter;
		SourceRef begin { 0, 1, 1 };
		SourceRef end;
		std::vector<LexError> errors;
		Lex lex = lexString(R"(
function(test_func Wut)
	message(${Wut})
	message(${ARGC})
	message(${ARGV1})
endfunction()

test_func("Hehe")
)",
		                    begin,
		                    end,
		                    errors);
		if (!errors.empty()) {
			for (auto& error : errors)
				printLexError(lex, error);
			return;
		}

		InterpreterState state { &lex };
		state.addDefaultFunctions();
		while (state.hasNext()) {
			state.next();
		}
	}
#endif
} // namespace MMake
