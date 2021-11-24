#include "Test.h"

#include <CommonCLI/Colors.h>
#include <ANSI/Graphics.h>

#include <iostream>

Tester& Tester::Get() {
	static Tester s_Instance;
	return s_Instance;
}

void Tester::test() {
	for (auto& tests : m_Tests) {
		using namespace CommonCLI;
		std::cout << Colors::Info << "=== " << tests.first << " ===\n";
		std::size_t numSuccess = 0;
		for (auto& test : tests.second) {
			bool result = test.m_Callback(*this);
			std::cout << Colors::Info << test.m_Name << " [";
			if (result) {
				std::cout << Colors::Success << "PASS";
				++numSuccess;
			} else {
				std::cout << Colors::Error << "FAIL";
			}
			std::cout << Colors::Info << "]\n";
		}
		
		std::cout << Colors::Info << "=== ";
		if (numSuccess >= tests.second.size() / 2) {
			if (numSuccess != tests.second.size())
				std::cout << Colors::Warn;
			else
				std::cout << Colors::Success;
			std::cout << numSuccess << "/" << tests.second.size() << " PASS";
		} else {
			std::cout << Colors::Error << (tests.second.size() - numSuccess) << "/" << tests.second.size() << " FAIL";
		}
		std::cout << Colors::Info << " ===\n";
	}
}

void Tester::addBoundedTest(const std::string& category, const std::string& name, TestCallback callback) {
	auto itr = m_Tests.find(category);
	if (itr == m_Tests.end())
		itr = m_Tests.insert({ category, {} }).first;
	itr->second.push_back({ name, callback });
}
