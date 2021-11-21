#pragma once

namespace MMake {
	void run();

#ifdef MMAKE_CMAKE_INTERPRETER
	void runCMake();
#endif
} // namespace MMake