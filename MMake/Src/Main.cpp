#include <CommonCLI/KeyValue/KVHandler.h>
#include <MMake/MMake.h>

#include <iostream>

int main(int argc, char** argv) {
	using namespace CommonCLI::KeyValue;
	Handler handler("MMake", "Build system generator", { 1, 0, 0, "", "alpha" });

	auto result    = handler.handle(argc, const_cast<const char**>(argv));
	auto& messages = result.getMessages();
	bool exit      = false;
	for (auto& message : messages) {
		if (message.isError()) {
			std::cerr << message.getStr() << '\n';
			exit = true;
		} else {
			std::cout << message.getStr() << '\n';
		}
	}
	if (exit)
		return 1;

		//MMake::run();
#ifdef MMAKE_CMAKE_COMPILER
	MMake::runCMake();
#endif
	return 0;
}