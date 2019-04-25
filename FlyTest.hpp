#pragma once

#include <functional>
#include <vector>
#include <utility>

class Condition;
class Section {
	friend class Conditon;
public:
	Section(Condition& condition)
	{
		condition.correspondSection._subSections.emplace_back(this);
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

	void markDone()
	{
		_selfDone = true;
	}
private:
	std::vector<Section*> _subSections;
	bool _selfDone{ false };
};

class Condition {
	// should also know the tree relationship(means share)
public:
	Section& correspondSection;
	Condition()
	: _section(nullptr)
	{

	}

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
		if (correspondSection._subSections.size() == 0)
		{
			correspondSection.markDone();
		}
	}

};

static std::vector<std::pair<Condition, std::function<void(Condition&)>>> tests{};

class RegisterTestCase {
public:
	RegisterTestCase(const std::function<void(Condition&)> testCase) // the const maybe not right
	{
		tests.emplace_back(std::make_pair(Condition{}, testCase));
	}
};

static RegisterTestCase testCase = (std::function<void(Condition&)>)[] (Condition& condition) {
	std::vector<int> a;
	a.push_back(1);
	static Section section{condition}; if (Condition condition = section) {
		// How to return?
	}
};

static
bool
allTest()
{
	for (auto& t : tests) {
		while (!t.first) { // t is false means t is not complete
			t.second(t.first);
		}
	}
}
