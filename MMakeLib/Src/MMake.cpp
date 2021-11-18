#include "MMake/MMake.h"

#include <CommonCLI/Colors.h>
#include <Premake/Defines.h>

#if PREMAKE_IS_CONFIG_DEBUG
	#define PICCOLO_ENABLE_DEBUG_LIB
#endif

extern "C" {
#include <Piccolo/debug/disassembler.h>
#include <Piccolo/include.h>
#include <Piccolo/stdlib/stdlib.h>
}

#include <cstdarg>

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

		piccolo_Package* package = piccolo_loadPackage(engine, "mmake.pic");
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
} // namespace MMake