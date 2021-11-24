#pragma once

#include <functional>
#include <vector>
#include <unordered_map>
#include <string_view>
#include <string>

class Tester;

using TestCallback = std::function<bool(Tester& tester)>;

struct Test {
public:
	std::string m_Name;
	TestCallback m_Callback;
};

class Tester {
public:
	static Tester& Get();
	
public:
	void test();
	
	template <class Func>
	void addTest(const std::string& category, const std::string& name, Func&& callback) {
		addBoundedTest(category, name, std::bind(std::forward<Func>(callback), std::placeholders::_1));
	}
	
	template <class Func, class T>
	void addTest(const std::string& category, const std::string& name, Func&& callback, T* obj) {
		addBoundedTest(category, name, std::bind(std::forward<Func>(callback), obj, std::placeholders::_1));
	}
	
	void addBoundedTest(const std::string& category, const std::string& name, TestCallback callback);
	
private:
	std::unordered_map<std::string, std::vector<Test>> m_Tests;
};
