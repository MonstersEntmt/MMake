#pragma once

#include "CMakeInterpreter/Utils/Source.h"

namespace CMakeLexer {
	class LexNodeType {
	public:
		virtual std::size_t toId()     = 0;
		virtual std::string toString() = 0;
	};

	struct LexNode {
	public:
	};

	struct Lex {
	public:
		Lex(CMake::ISource* source);

	private:
		CMake::ISource* m_Source;
	};
} // namespace CMakeLexer