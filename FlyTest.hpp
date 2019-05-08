#pragma once

#include <functional>
#include <vector>
#include <exception>
using std::runtime_error;

class Condition;
class Section {
	// Section don't have the data detail rely on Condition class, but Condition do.
	// So we can pass a condition as parameter in TESTCASE not reference without error
	friend class Condition;
public:
	Section(Condition& condition);
	Section() : _father(nullptr)
	{}

	bool state()
	{
		// TODO below code should be reduce
		if (_state) {
			// if this call end, no branch need to be run.
			return true;
		} else {
			return allBranchDone();
		}
	}

	bool allBranchDone()
	{
		if (_subSections.empty()) {
			return _selfDone;
		}

		for (auto s : _subSections) {
			if (!s->allBranchDone()) {
				return false;
			}
		}
		return true;
	}

	void markDone()
	{
		_selfDone = true;
	}

	// void turnOffState() const
	// {
	// 	_state = true;
	// }
	//
	// void turnOnState() const
	// {
	// 	_state = false;
	// }

	bool subSectionEmpty() const
	{
		return _subSections.empty();
	}

private:
	bool _selfDone{ false };
	// bool& _state;
	Section* const _father;
	std::vector<Section*> _subSections;
};

class Condition {
private:
	bool& _state;
public:
	Section& correspondSection;

	Condition(Section& section)
		: correspondSection(section)
	{}

	Condition(Section& section, bool& state)
		: correspondSection(section), _state(state)
	{}

	operator bool() const
	{
		// ! to let it fit the if condition
		return !_state && !correspondSection.allBranchDone();
	}

	~Condition()
	{
		// means it's leaf
		if (!_state && correspondSection._subSections.empty()) {
			correspondSection.markDone();
			// correspondSection.turnOffState();
			_state = true;
		}
	}

};

inline Section::Section(Condition &condition)
	// : _state(condition.correspondSection._state),
	: _father(&condition.correspondSection)
{
	// register sub-section
	condition.correspondSection._subSections.emplace_back(this);

}

static std::vector<std::function<void(Condition&)>> _tests{};

class RegisterTestCase {
public:
	RegisterTestCase(std::function<void(Condition&)> testCase) // the const maybe not right
	{
		// Here exist a problem
		_tests.emplace_back(testCase);
	}
};

// example
//static RegisterTestCase testCase = (std::function<void(Condition&)>)[] (Condition& condition) {
//	// do something
//
//	static Section section1{ condition }; if (Condition condition = section1) {
//		// How to return?
//	}
//};

static
void
allTest()
{
	for (auto& t : _tests) {
		Section testFunc;
		while (!testFunc.allBranchDone()) { // t is false means t is not complete
			auto onceState = false;
			Condition condition{ testFunc, onceState}; // for once run
			t(condition); // t(std::move(condition));
			// testFunc.turnOnState(); // call once will set the _funcState to true

			// for null TESTCASE:
			// if (testFunc.subSectionEmpty()) {
			// 	testFunc.markDone();
			// }
		}
	}
}

#define PRIMITIVE_CAT(A, B) A##B
#define CAT(A, B) PRIMITIVE_CAT(A, B)

#define TESTCASE(DESCRIPTION) static RegisterTestCase CAT(testcase, __LINE__) = (std::function<void(Condition)>)[] (Condition condition)

#define SECTION(DESCRIPTION) static Section CAT(section, __LINE__) { condition }; if (Condition condition = CAT(section, __LINE__) )

// 我希望一个 TESTCASE 有两个状态，内部有个本次执行完毕的状态，外部有个标识所有 branch 执行完的
// 的状态

// testFunc.allBranchDone() // external state
// condition // internal state
