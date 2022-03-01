#include "MMake/MMake.h"

#include <CommonCLI/Colors.h>
#include <CommonCLI/Core.h>

#include <CMakeLexer/Lexer.h>
#include <CommonLexer/LexerLexer.h>

extern "C"
{
#include <Piccolo/debug/disassembler.h>
#include <Piccolo/include.h>
#include <Piccolo/stdlib/picStdlib.h>
}

#include <cstdarg>

#include <filesystem>
#include <iostream>
#include <regex>
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

	std::string EscapeString(const std::string& str)
	{
		return std::regex_replace(str, std::regex { "\"" }, "\\\"");
	}

	void PrintLexNode(const CommonLexer::LexNode& node, std::vector<bool>& layers, bool end = true)
	{
		std::ostringstream str;

		for (auto layer : layers)
		{
			if (layer)
				str << "\xE2\x94\x82 ";
			else
				str << "  ";
		}

		if (end)
			str << "\xE2\x94\x94\xE2\x94\x80";
		else
			str << "\xE2\x94\x9C\xE2\x94\x80";

		layers.push_back(!end);

		auto& span = node.getSpan();
		str << node.getRule() << " (" << span.m_Begin.m_Line << ":" << span.m_Begin.m_Column << " -> " << span.m_End.m_Line << ":" << span.m_End.m_Column << ')';
		if (span.m_Begin.m_Line == span.m_End.m_Line)
		{
			std::string s = node.getSource()->getSpan(span);
			if (s.find_first_of('\n') < s.size())
				str << '\n';
			else
				str << " : \"" << EscapeString(s) << "\"\n";
		}
		else
			str << '\n';
		std::cout << str.str();
		auto& children = node.getChildren();
		for (std::size_t i = 0; i < children.size(); ++i)
			PrintLexNode(children[i], layers, i >= children.size() - 1);

		layers.pop_back();
	}

	void PrintLex(const CommonLexer::Lex& lex)
	{
		std::vector<bool> layers;
		PrintLexNode(lex.getRoot(), layers);
	}

	void RunCMake()
	{
		CommonLexer::StringSource lexerSource { R"(
File?:         LineElement*
LineElement?:  Line? '([ \t]*#.*)?[ \t]*\n?'
Line:          NodeRule
               NonNodeRule
               CallbackRule
NodeRule:      Identifier '[ \t]*:[ \t]*' Value
NonNodeRule:   Identifier '[ \t]*\?[ \t]*:[ \t]*' Value
CallbackRule:  Identifier '[ \t]*![ \t]*:'
Identifier:    '[A-Za-z_][A-Za-z0-9_]*'
Value?:        Branch
               OneLineValue
OneLineValue?: Combination
               Or
               NonMultValue
NonMultValue?: ZeroOrMore
               OneOrMore
               Optional
               BasicValue
BasicValue?:   Group
               NamedGroup
               Reference
               Identifier
               Regex
Regex:         '\'(?:[^\'\\\n]|\.|\\.)*\''
ZeroOrMore:    BasicValue '\*'
OneOrMore:     BasicValue '\+'
Optional:      BasicValue '\?'
Combination:   NonMultValue ('[ \t]+' NonMultValue)+
Or:            NonMultValue ('[ \t]*\|[ \t]*' NonMultValue)+
Branch:        OneLineValue ('\n[ \t]*' OneLineValue)+
Group:         '\([ \t]*' OneLineValue '[ \t]*\)'
NamedGroup:    '\([ \t]*<[ \t]*' Identifier '[ \t]*>[ \t]*:[ \t]*' OneLineValue '[ \t]*\)'
Reference:     '\\' Identifier
)" };

		CommonLexer::LexerLexer lexerLexer;

		auto lexerLex = lexerLexer.lexSource(&lexerSource);
		if (!lexerLex.getMessages().empty())
		{
			for (auto& message : lexerLex.getMessages())
				PrintMessage(message, &lexerSource);
			return;
		}

		PrintLex(lexerLex);

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

		PrintLex(lex);
	}
} // namespace MMake
