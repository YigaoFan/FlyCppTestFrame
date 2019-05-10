#include <utility>

#pragma once

#include <functional>
#include <vector>
#include <string>
#include <tuple>
#include <iostream>
using std::string;
using std::vector;
using std::tuple;
using std::make_tuple;
using std::cout;
using std::endl;
using std::ostream;
using std::flush;

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

static vector<std::function<void(Condition, bool&)>> _tests{};

class RegisterTestCase {
public:
	RegisterTestCase(std::function<void(Condition, bool&)> testCase) // the const maybe not right
	{
		_tests.emplace_back(testCase);
	}
};

class AssertionFailure {
	// TODO some failure info
private:
	string _expression;
	string _fileName;
	int16_t _line;
public:
	AssertionFailure(string expression, string fileName, int16_t line)
		: _expression(std::move(expression)), _fileName(std::move(fileName)), _line(line)
	{}

	string fileName() const
	{
		return _fileName;
	}

	int16_t line() const
	{
		return _line;
	}
};

class SectionRouteTrack {
private:
	vector<tuple<string, int16_t, string>> _previousRoutePart{};
	tuple<string, int16_t, string> _currentSection;
	bool _currentSectionValid{ false };
public:
	SectionRouteTrack() = default;

	void append(const string& fileName, int16_t line, const string& sectionName)
	{
		if (_currentSectionValid) {
			_previousRoutePart.emplace_back(_currentSection);
		}
		_currentSection = std::move(make_tuple(fileName, line, sectionName));
		_currentSectionValid = true;
	}

	void log(int8_t initialIndentation = 0, ostream& out = cout) const
	{
		out << "Testcase state: \n";

		if (!_currentSectionValid) { return; }

		for (const auto &s : _previousRoutePart) {
			showNSpace(initialIndentation, out);
			showSectionInfo(s, out) << endl;
			++initialIndentation;
		}
		showNSpace(initialIndentation, out) << "-> ";
		showSectionInfo(_currentSection, out) << endl;
	}

private:
	static ostream& showNSpace(int8_t num, ostream& out)
	{
		for (auto i = 0; i < num; ++i) {
			out << ' ';
		}
		out.flush();
		return out;
	}

	static ostream& showSectionInfo(decltype(_currentSection) sectionInfo, ostream& out)
	{
		for (auto i = 0; i < 3; ++i) {
			out
			<< std::get<0>(sectionInfo) << ": "
			<< std::get<1>(sectionInfo) << ": "
			<< std::get<2>(sectionInfo);
		}
		out.flush();
		return out;
	}

//	void reset()
//	{
//		_previousRoutePart.clear();
//		_currentSectionValid = false;
//	}
};

static
void
allTest()
{
	for (auto& t : _tests) {
		Section testFunc;
		while (!testFunc.allBranchDone()) { // t is false means t is not complete
			auto onceState = false;
			SectionRouteTrack currentSectionTrack;
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
// below __FILE__ and __LINE__ maybe used wrong

#define ASSERT(EXP) do { if (!(EXP)) { throw AssertionFailure(#EXP, __FILE__ , __LINE__ ); } } while(0)

// how to rethrow this exption?
#define ASSERT_THROW(TYPE, EXP) 	\
	do {                        	\
    	try {                   	\
			(EXP);              	\
    	} catch (TYPE e) { }    	\
		  catch (...) { throw; }	\
		throw;						\
	} while(0)





