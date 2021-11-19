#include "MMake/MMake.h"

#include <CommonCLI/Colors.h>
#include <Premake/Defines.h>

#ifdef MMAKE_CMAKE_COMPILER
	#include <CMakeCompiler/Lexer.h>
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

#ifdef MMAKE_CMAKE_COMPILER
	static void printLexNodeType(CMakeCompiler::LexNodeType type) {
		using namespace CMakeCompiler;
		switch (type) {
		case LexNodeType::File:
			std::cout << "File";
			break;
		case LexNodeType::FileElement:
			std::cout << "FileElement";
			break;
		case LexNodeType::LineEnding:
			std::cout << "LineEnding";
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
		case LexNodeType::Argument:
			std::cout << "Argument";
			break;
		case LexNodeType::BracketArgument:
			std::cout << "BracketArgument";
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
		case LexNodeType::QuotedContinuation:
			std::cout << "QuotedContinuation";
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

	static void printSourceRef(CMakeCompiler::SourceRef sourceRef) {
		std::cout << sourceRef.m_Index << ": " << sourceRef.m_Line << ", " << sourceRef.m_Column;
	}

	static void printLexNode(const CMakeCompiler::LexNode& node, std::size_t tabs = 0) {
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

	static void printLex(const CMakeCompiler::Lex& lex) {
		std::cout << "Lex:\n";
		printLexNode(lex.m_Root, 1);
	}

	void runCMake() {
		using namespace CMakeCompiler;
		SourceRef begin { 0, 1, 1 };
		SourceRef end;
		std::vector<LexError> errors;
		Lex lex = lexString(R"(include(${CMAKE_SOURCE_DIR}/scripts/CMakeUtils.cmake)

file(GLOB_RECURSE SOURCE_FILES
    Source/Surge/*.cpp
    Source/Surge/*.hpp
    Source/SurgeReflect/*.hpp
    Source/SurgeReflect/*.cpp
    Vendor/stb/stb_image.cpp
)

set(INCLUDE_DIRS
    Source
    Source/SurgeReflect/Include
    Vendor
    Vendor/fmt/Include
    Vendor/glm
    Vendor/Vulkan-Headers/Include
    Vendor/VulkanMemoryAllocator/Include
    Vendor/volk
    Vendor/shaderc/Include
    Vendor/SPIRV-Cross/Include
    Vendor/stb
    Vendor/ImGui
    Vendor/Optick/Include
    Vendor/entt/Include
    Vendor/json/Include
    Vendor/assimp/Include
    Vendor/FontAwesome
)

set(LIB_LINKS
    fmt
    volk
    SPIRV-Cross
    ImGui
    Optick
    ${CMAKE_SOURCE_DIR}/Engine/Vendor/shaderc/Lib/shaderc_shared.lib
    ${CMAKE_SOURCE_DIR}/Engine/Vendor/assimp/Lib/assimp-vc142-mt.lib
)

add_library(Surge STATIC ${SOURCE_FILES})
target_include_directories(Surge PUBLIC ${INCLUDE_DIRS})
target_link_libraries(Surge PUBLIC ${LIB_LINKS})

# Add Precompiled Header
target_precompile_headers(Surge PRIVATE "Source/Pch.hpp")

GroupSourcesByFolder(Surge)

set_property(TARGET Surge PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_target_properties(Surge PROPERTIES FOLDER Engine)

if (WIN32)
set(PLATFORM_COMPILE_DEFS VK_USE_PLATFORM_WIN32_KHR NOMINMAX)
endif (WIN32)

target_compile_definitions(Surge

    PUBLIC
    "_CRT_SECURE_NO_WARNINGS"
    "GLM_FORCE_DEPTH_ZERO_TO_ONE"
    "GLM_FORCE_RADIANS"
    
    ${PLATFORM_COMPILE_DEFS}

    $<$<CONFIG:Debug>:SURGE_DEBUG>
    $<$<CONFIG:Release>:SURGE_RELEASE>
)
)",
		                    begin,
		                    end,
		                    errors);
		if (!errors.empty()) {
			for (auto& error : errors)
				std::cerr << error.m_At.m_Line << ", " << error.m_At.m_Column << ": " << error.m_Message << "\n";
			return;
		}
		printLex(lex);
	}
#endif
} // namespace MMake