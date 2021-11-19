#pragma once

namespace MMake {
	void run();

#ifdef MMAKE_CMAKE_COMPILER
	void runCMake();
#endif
} // namespace MMake