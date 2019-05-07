#pragma once

#include <functional>
#include <vector>
#include <exception>
using std::runtime_error;

class Condition;
class Section {
    friend class Condition;
public:
    Section(Condition& condition);
    Section(bool& state) : _state(state), _father(nullptr)
    {}

    bool state()
    {
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
#ifdef FALSE
                if (_selfDone) {
                    throw runtime_error
                    ("When a sub branch is not done, _selfDone of current branch is not be set to false");
                }
#endif
                return false;
            }
        }
#ifdef FALSE
        if (_selfDone) {
            throw runtime_error
            ("When all sub branches are done, _selfDone of current branch is not be set to true");
        }
#endif
        return true; // this maybe have some problem
    }

    void markDone()
    {
        _selfDone = true;
        //		if (_father != nullptr) {
        //			_father->checkSelfStatus();
        //		}
    }

    void checkSelfStatus()
    {
        auto subStatus{ true };
        for (const auto &s : _subSections) {
            subStatus &= s->allBranchDone();
        }

        if (subStatus) {
            markDone();
        }
    }

    void turnOffState() const
    {
        // TODO why this member function can be const
        _state = true;
    }

    void turnOnState() const
    {
        _state = false;
    }

private:
    bool _selfDone{ false };
    bool& _state;
    Section* const _father;
    std::vector<Section*> _subSections;
};

class Condition {
public:
    Section& correspondSection;

    Condition(Section& section)
        : correspondSection(section)
    {}

    operator bool() const
    {
        // ! to let it fit the if condition
        return !correspondSection.state();
    }

    ~Condition()
    {
        // means it's leaf
        if (correspondSection._subSections.empty() && !correspondSection.state()) {
            correspondSection.markDone();
            correspondSection.turnOffState();
        }
    }

};

inline Section::Section(Condition &condition)
    : _state(condition.correspondSection._state),
    _father(&condition.correspondSection)
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
        auto funcState = false;
        Section testFunc{ funcState };
        Condition condition{ testFunc };
        while (!testFunc.allBranchDone()) { // t is false means t is not complete
            t(condition);
            testFunc.turnOnState(); // call once will set the _funcState to true
        }
    }
}

#define PRIMITIVE_CAT(A, B) A##B
#define CAT(A, B) PRIMITIVE_CAT(A, B)

#define TESTCASE(DESCRIPTION) static RegisterTestCase CAT(testcase, __LINE__) = (std::function<void(Condition&)>)[] (Condition& condition)

#define SECTION(DESCRIPTION) static Section CAT(section, __LINE__) { condition }; if (Condition condtion = CAT(section, __LINE__) )
