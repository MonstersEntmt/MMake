#include "MMake/MMake.h"

#include <CommonCLI/Colors.h>
#include <Premake/Defines.h>

#include <CMakeLexer/Lexer.h>

#if BUILD_IS_CONFIG_DEBUG
#define PICCOLO_ENABLE_DEBUG_LIB
#endif

extern "C"
{
#include <Piccolo/debug/disassembler.h>
#include <Piccolo/include.h>
#include <Piccolo/stdlib/stdlib.h>
}

#include <cstdarg>

#include <filesystem>
#include <iostream>
#include <sstream>

namespace MMake
{
	static void PrintPiccoloError(const char* format, std::va_list args)
	{
		std::ostringstream error;
		error << CommonCLI::Colors::Error;
		int   size    = std::vsnprintf(nullptr, 0, format, args);
		char* buf     = new char[size + 2];
		buf[size + 1] = '\0';
		std::vsnprintf(buf, size + 1, format, args);
		error << buf;
		delete[] buf;
		std::cout << error.str();
	}

	void Run()
	{
		piccolo_Engine* engine = new piccolo_Engine();
		piccolo_initEngine(engine, &PrintPiccoloError);
		piccolo_addIOLib(engine);
		piccolo_addTimeLib(engine);
#if BUILD_IS_CONFIG_DEBUG
		piccolo_addDebugLib(engine);
#endif

		std::filesystem::path mainPicFile = std::filesystem::current_path() / "mmake.pic";
		piccolo_Package*      package     = piccolo_loadPackage(engine, mainPicFile.string().c_str());
		if (package->compilationError)
		{
			piccolo_freeEngine(engine);
			delete engine;
			return;
		}

		if (!piccolo_executePackage(engine, package))
		{
			piccolo_enginePrintError(engine, "Runtime error.\n");
		}

		piccolo_freeEngine(engine);
	}

	void PrintMessage(const CommonLexer::Message& message, CommonLexer::ISource* source)
	{
		std::ostringstream str;
		switch (message.getSeverity())
		{
		case CommonLexer::EMessageSeverity::Warning:
			str << CommonCLI::Colors::Warn << "CMakeLexer Warning: ";
			break;
		case CommonLexer::EMessageSeverity::Error:
			str << CommonCLI::Colors::Warn << "CMakeLexer Error: ";
			break;
		}
		str << message.getMessage() << ANSI::GraphicsForegroundDefault << '\n';

		auto& span  = message.getSpan();
		auto  lines = source->getLines(span.m_Begin.m_Line, span.m_End.m_Line - span.m_Begin.m_Line);
		for (std::size_t i = 0; i < lines.size(); ++i)
		{
			auto& line = lines[i];
			str << line << '\n';
			if (i == 0)
				str << CommonCLI::Colors::Note << std::string(span.m_Begin.m_Column - 1, ' ') << '^' << std::string(line.size() - span.m_Begin.m_Column, '~');
			else if (i == lines.size() - 1)
				str << CommonCLI::Colors::Note << std::string(span.m_End.m_Column, '~');
			else
				str << CommonCLI::Colors::Note << std::string(line.size(), '~');
			str << ANSI::GraphicsBackgroundDefault << '\n';
		}

		std::cout << str.str();
	}

	void PrintLex(const CommonLexer::Lex& lex, CommonLexer::ISource* source)
	{
		
	}

	void RunCMake()
	{
		CommonLexer::StringSource source { R"(
macro(test_macro)
	function(test_func)
		macro(another_macro)
			message("He")
		endmacro()
		message("Cool")
		another_macro()
	endfunction()
	message("World")
	test_func()
	return()
endmacro()

message("Hello")
test_macro()
message("Skipped")
)" };

		CMakeLexer::Lexer lexer;

		auto lex = lexer.lexSource(&source);

		if (!lex.getMessages().empty())
		{
			for (auto& message : lex.getMessages())
				PrintMessage(message, &source);
			return;
		}

		PrintLex(lex, &source);
	}
} // namespace MMake
