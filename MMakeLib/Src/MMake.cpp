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

#include <chrono>
#include <filesystem>
#include <iostream>
#include <regex>
#include <sstream>

namespace Generated
{
	CommonLexer::Lex lexSource(CommonLexer::ISource* source, CommonLexer::SourceSpan span);
	CommonLexer::Lex lexSource(CommonLexer::ISource* source);
} // namespace Generated

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
			str << CommonCLI::Colors::Warn << "CommonLexer Warning ";
			break;
		case CommonLexer::EMessageSeverity::Error:
			str << CommonCLI::Colors::Error << "CommonLexer Error ";
			break;
		}
		auto point       = message.getPoint();
		auto pointLine   = point.getLine(source);
		auto pointColumn = point.getColumn(source);
		str << '(' << pointLine << ':' << pointColumn << "): ";
		str << message.getMessage() << ANSI::GraphicsForegroundDefault << '\n';

		auto span        = message.getSpan();
		auto lines       = source->getLines(span);
		auto beginLine   = span.m_Begin.getLine(source);
		auto beginColumn = span.m_Begin.getColumn(source);
		auto endLine     = span.m_End.getLine(source);
		auto endColumn   = span.m_End.getColumn(source);
		for (std::size_t i = 0; i < lines.size(); ++i)
		{
			std::string ln   = std::to_string(beginLine + i) + ": ";
			auto&       line = lines[i];
			if (line.empty())
				continue;
			str << ln << line << '\n'
			    << std::string(ln.size(), ' ');
			str << CommonCLI::Colors::Note;

			if (i == 0)
			{
				str << std::string(beginColumn - 1, ' ');

				if (beginLine + i == pointLine)
				{
					str << std::string(pointColumn - beginColumn, '~') << '^';

					if (endLine == beginLine)
						str << std::string(endColumn - pointColumn, '~');
					else
						str << std::string(line.size() - pointColumn, '~');
				}
				else
				{
					str << std::string(line.size() - beginColumn, '~');
				}
			}
			else if (i == lines.size() - 1)
			{
				if (beginLine + i == pointLine)
					str << std::string(pointColumn - 1, '~') << '^' << std::string(endColumn - pointColumn, '~');
			}
			else
			{
				str << std::string(line.size(), '~');
			}
			str << ANSI::GraphicsForegroundDefault << '\n';
		}

		std::cout << str.str();
	}

	std::string EscapeString(const std::string& str)
	{
		return std::regex_replace(str, std::regex { "\"" }, "\\\"");
	}

	std::size_t UTF8Codepoints(const std::string& str)
	{
		auto itr = str.begin();
		auto end = str.end();

		std::size_t count = 0;
		while (itr != end)
		{
			std::uint8_t c = *itr;
			if (c < 0b1000'0000U)
				++itr;
			else if (c < 0b1100'0000U)
				itr += 2;
			else if (c < 0b1110'0000U)
				itr += 3;
			else if (c < 0b1111'0000U)
				itr += 4;
			++count;
		}
		return count;
	}

	void PrintLexNode(const CommonLexer::Node& node, std::vector<std::vector<std::string>>& lines, std::vector<bool>& layers, bool end = true)
	{
		{
			std::vector<std::string> line;
			std::ostringstream       str;

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

			auto span = node.getSpan();
			str << node.getRule();
			line.push_back(str.str());
			str = {};

			auto beginLine   = span.m_Begin.getLine(node.getSource());
			auto beginColumn = span.m_Begin.getColumn(node.getSource());
			auto endLine     = span.m_End.getLine(node.getSource());
			auto endColumn   = span.m_End.getColumn(node.getSource());
			str << '(' << beginLine << ':' << beginColumn << " -> " << endLine << ':' << endColumn << ')';
			line.push_back(str.str());
			str = {};

			if (beginLine == endLine)
			{
				std::string s = node.getSource()->getSpan(span);
				if (s.find_first_of('\n') >= s.size())
				{
					str << "= \"" << EscapeString(s) << '"';
					line.push_back(str.str());
				}
			}
			lines.push_back(std::move(line));
		}

		auto& children = node.getChildren();
		for (std::size_t i = 0; i < children.size(); ++i)
			PrintLexNode(children[i], lines, layers, i >= children.size() - 1);

		layers.pop_back();
	}

	void PrintLex(const CommonLexer::Lex& lex)
	{
		std::vector<std::vector<std::string>> lines;
		std::vector<bool>                     layers;
		PrintLexNode(lex.getRoot(), lines, layers);

		std::vector<std::size_t> sizes;
		for (auto& line : lines)
		{
			if ((line.size() - 1) > sizes.size())
				sizes.resize(line.size() - 1, 0);

			for (std::size_t i = 0; i < line.size() - 1; ++i)
			{
				auto&       column     = line[i];
				std::size_t codepoints = UTF8Codepoints(column);
				if (codepoints > sizes[i])
					sizes[i] = codepoints;
			}
		}

		std::ostringstream str;
		for (auto& line : lines)
		{
			for (std::size_t i = 0; i < line.size(); ++i)
			{
				auto& column = line[i];
				str << column;
				if (i < line.size() - 1)
					str << std::string(sizes[i] - UTF8Codepoints(column) + 1, ' ');
			}
			str << '\n';
		}
		std::cout << str.str();
	}

	void RunCMake()
	{
		CommonLexer::StringSource lexerSource(R"kekw(!MainRule = File;

File?:        FileElement*;
FileElement?: CommandInvocation LineEnding;
              (BracketComment | Space)* LineEnding;
LineEnding?:  LineComment? Newline;
Space?:       '[ \t]+';
Newline?:     "\n";

CommandInvocation:  Space* Identifier Space* "(" Arguments ")";
Identifier:         '[A-Za-z_][A-Za-z0-9_]*';
Arguments:          Argument? SeparatedArguments*;
SeparatedArguments: Separation+ Argument?;
					Separation* "(" Arguments ")";
Separation?:        Space;
                    LineEnding;

Argument: BracketArgument;
          QuotedArgument;
          UnquotedArgument;

BracketArgument: BracketOpen BracketContent BracketClose;
BracketOpen?:    "[" (<BracketCount>: '='*) "[";
BracketContent:  (("]" ~\BracketCount) | '[^\\]]' | Newline)*;
BracketClose?:   "]" \BracketCount "]";

QuotedArgument:     "\"" QuotedElement* "\"";
QuotedElement:      EscapeSequence;
                    QuotedContinuation;
                    (newline | '[^"\\]')+;
QuotedContinuation: "\\" Newline;

UnquotedArgument: UnquotedElement+;
                  UnqoutedLegacy;
UnqoutedElement:  EscapeSequence;
                  '(?:[^\\s()#"\\\\])+';
UnquotedLegacy!;

EscapeSequence: '\\(?:[^A-Za-z0-9;]|[trn;])';

LineComment:    '#(?!\\[=*\\[).*';
BracketComment: "#" BracketArgument;)kekw");

		CommonLexer::LexerLexer lexerLexer;

		auto begin    = std::chrono::high_resolution_clock::now();
		auto lexerLex = lexerLexer.lexSource(&lexerSource);
		auto end      = std::chrono::high_resolution_clock::now();
		if (!lexerLex.getMessages().empty())
			for (auto& message : lexerLex.getMessages())
				PrintMessage(message, &lexerSource);

		PrintLex(lexerLex);

		std::cout << std::format("\nTime: {:%S} S\n", end - begin);

		auto result = lexerLexer.createCPPLexer(lexerLex);
		if (!result.m_Messages.empty())
			for (auto& message : result.m_Messages)
				PrintMessage(message, &lexerSource);

		auto          cpp = lexerLexer.compileLexer(result, "Generated");
		std::ofstream f { "Out.cpp" };
		if (f)
		{
			f << cpp;
			f.close();
		}

		/*std::cout << "\n^ Original Lex ^\nv New Lex v\n\n";

		auto lexBegin = std::chrono::high_resolution_clock::now();
		auto lex      = Generated::lexSource(&lexerSource);
		auto lexEnd   = std::chrono::high_resolution_clock::now();
		if (!lex.getMessages().empty())
			for (auto& message : lex.getMessages())
				PrintMessage(message, &lexerSource);

		PrintLex(lex);

		std::cout << std::format("\nOriginal Lex: {:%S} S\nNew Lex: {:%S} S\n", end - begin, lexEnd - lexBegin);*/

		/*CommonLexer::StringSource source { R"(
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

		PrintLex(lex);*/
	}
} // namespace MMake
