#pragma once

#include <functional>
#include <vector>
#include <utility>

class Condition;
class Section {
public:
	Section(Condition& condition)
	{
		// todo
	}
	bool state()
	{
		if (_subSections.size() == 0) {
			return _selfDone;
		}
		// else traverse the sub-tree
		for (auto s : _subSections) {
			if (!s->state()) {
				return false;
			}
		}
		return true;
	}

private:
	std::vector<Section*> _subSections;
	bool _selfDone{ false };
};

class Condition {
public:
	Condition()
	: _section(nullptr)
	{

	}

	Condition(Section& section)
		: _section(section)
	{}

	operator bool()
	{
		return _section.state();
	}

private:
	Section& _section;

};

static std::vector<std::pair<Condition, std::function<void(Condition&)>>> tests{};

class RegisterTestCase {
public:
	RegisterTestCase(std::function<void(Condition&)> testCase)
	{
		tests.emplace_back(std::make_pair(Condition{}, testCase));
	}
};

static RegisterTestCase testCase = (std::function<void(Condition&)>)[] (Condition& condition) {
	std::vector<int> a;
	a.push_back(1);
	static Section section{condition}; if (Condition condition = section) {

	}
};

static bool allTest()
{
	for (auto& t : tests) {
		while (!t.first) { // t is false means t is not complete
			t.second(t.first);
		}
	}
}
