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
	Section() {}

//	bool state()
//	{
//		// TODO below code should be reduce
//		if (_state) {
//			// if this call end, no branch need to be run.
//			return true;
//		} else {
//			return allBranchDone();
//		}
//	}

	bool allBranchDone() const
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

//	bool subSectionEmpty() const
//	{
//		return _subSections.empty();
//	}

private:
	bool _selfDone{ false };
	std::vector<Section*> _subSections{};
};

class Condition {
private:
	bool& _state;
public:
	Section& correspondSection;

	Condition(Section& section, bool& state)
		: correspondSection(section), _state(state)
	{}

	operator bool() const
	{
		// ! to let it fit the if condition
		if (_state) {
			return false;
		} else if (correspondSection.allBranchDone()){
			return false;
		}
		return true;
	}

	~Condition()
	{
		if (*this) {
			// means it's leaf
			if (correspondSection._subSections.empty()) {
				correspondSection.markDone();
				_state = true;
			}
		}
	}
};

inline Section::Section(Condition &condition)
{
	// register sub-section
	condition.correspondSection._subSections.emplace_back(this);

}

static std::vector<std::function<void(Condition, bool&)>> _tests{};

class RegisterTestCase {
public:
	RegisterTestCase(std::function<void(Condition, bool&)> testCase) // the const maybe not right
	{
		_tests.emplace_back(testCase);
	}
};

class AssertionFailure {
	// TODO some failure info
};

static
void
allTest()
{
	for (auto& t : _tests) {
		Section testFunc;
		while (!testFunc.allBranchDone()) { // t is false means t is not complete
			auto onceState = false;
			try {
				t(Condition(testFunc, onceState), onceState); // t(std::move(condition));
			} catch (AssertionFailure f) {
				// TODO show failure info
			} catch (...) {
				// report uncaught exception
			}

		}
	}
}
// 我希望一个 TESTCASE 有两个状态，内部有个本次执行完毕的状态，外部有个标识所有 branch 执行完的的状态
// testFunc.allBranchDone() external state
// condition internal state
#define PRIMITIVE_CAT(A, B) A##B
#define CAT(A, B) PRIMITIVE_CAT(A, B)

#define TESTCASE(DESCRIPTION) static RegisterTestCase CAT(testcase, __LINE__) = (std::function<void(Condition, bool&)>)[] (Condition condition, bool& onceState)

#define SECTION(DESCRIPTION) static Section CAT(section, __LINE__) { condition }; if (Condition condition{ CAT(section, __LINE__) , onceState })

#define ASSERT(EXP) 		\
		try {				\

		}

#define ASSERT_THROW(TYPE, EXP)

template <typename T>
void
func()
{
	try {

	} catch (T e) {

	}

	// no exception caught
}



