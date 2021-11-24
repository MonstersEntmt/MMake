#include "FileIO.h"

#include <fstream>

std::string readFile(const std::filesystem::path& file) {
	std::ifstream stream{ file, std::ios::ate };
	if (stream.is_open()) {
		std::string str(stream.tellg(), '\0');
		stream.seekg(0);
		stream.read(str.data(), str.size());
		stream.close();
		return str;
	}
	return {};
}
