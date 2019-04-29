#pragma once

#include <functional>
#include <vector>
#include <utility>

class Condition;
class Section {
	friend class Condition;
public:
	Section(Condition& condition);
	Section(bool state)
	: _state(state){}

	bool state()
	{
		if (_subSections.empty()) {
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

	bool allBranchDone()
	{
		for (auto s : _subSections) {
			if (!s->state()) {
				return false;
			}
		}
		return true;
	}

	void markDone()
	{
		_selfDone = true;
	}
private:
	std::vector<Section*> _subSections;
	bool _selfDone{ false };
	bool& _state; // just represent once call end or not
};

class Condition {
public:
	Section& correspondSection;

	Condition(Section& section)
		: correspondSection(section)
	{}

	operator bool()
	{
		return correspondSection.state();
	}

	~Condition()
	{
		// means it's leaf
		if (correspondSection._subSections.empty())
		{
			correspondSection.markDone();
		}
	}

};

Section::Section(Condition &condition)
{
	condition.correspondSection._subSections.emplace_back(this);
}

static std::vector<std::function<void(Condition&)>> _tests{};

class RegisterTestCase {
public:
	RegisterTestCase(const std::function<void(Condition&)> testCase) // the const maybe not right
	{
		// Here exist a problem
		_tests.emplace_back(testCase);
	}
};

// example
static RegisterTestCase testCase = (std::function<void(Condition&)>)[] (Condition& condition) {
	// do something
	std::vector<int> a;
	a.push_back(1);

	static Section section1{ condition }; if (Condition condition = section1) {
		// How to return?
	}
};

static
bool
allTest()
{
	for (auto& t : _tests) {
		bool state = false;
		Section testFunc{ state };
		Condition condition{ testFunc };
		while (!testFunc.allBranchDone()) { // t is false means t is not complete
			t(condition);
			state = false; // call once will set the state to true
		}
	}
}
