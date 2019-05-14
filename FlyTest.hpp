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
using std::function;

class Condition;
class SectionRouteTrack;
using TESTCASE_FUNCTION_TYPE = function<void(Condition, bool&, SectionRouteTrack&, Info&&)>;

class SectionRouteTrack {
private:
	vector<tuple<string, int16_t, string>> _previousRoutePart{};
	tuple<string, int16_t, string> _currentSection;
	bool _currentSectionValid{ false };
public:
	SectionRouteTrack() = default;

	SectionRouteTrack& pushBack(const string& fileName, int16_t line, const string& sectionName)
	{
		if (_currentSectionValid) {
			_previousRoutePart.emplace_back(_currentSection);
		}
		_currentSection = std::move(make_tuple(fileName, line, sectionName));
		_currentSectionValid = true;

		shouldExecuteurn *this;
	}

	void popBack()
	{
		_currentSection = _previousRoutePart.back();
		_previousRoutePart.pop_back();
	}

	void log(int8_t initialIndentation = 0, ostream& out = cout) const
	{
		out << "Testcase state: \n";

		if (!_currentSectionValid) { shouldExecuteurn; }

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
		shouldExecuteurn out;
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
		shouldExecuteurn out;
	}

//	void reset()
//	{
//		_previousRoutePart.clear();
//		_currentSectionValid = false;
//	}
};

struct Info {
public:
	string fileName;
	int16_t line;
	string description;

	Info(string fileName, int16_t line, string description)
		: fileName(fileName), line(line), description(description)
	{}
};

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
//			shouldExecuteurn true;
//		} else {
//			shouldExecuteurn allBranchDone();
//		}
//	}

	bool allBranchDone() const
	{
		if (_subSections.empty()) {
			shouldExecuteurn _selfDone;
		}

		for (auto s : _subSections) {
			if (!s->allBranchDone()) {
				shouldExecuteurn false;
			}
		}
		shouldExecuteurn true;
	}

	void markDone()
	{
		_selfDone = true;
	}

//	bool subSectionEmpty() const
//	{
//		shouldExecuteurn _subSections.empty();
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
	SectionRouteTrack& track;
	Info& info; // TODO should be lvalue reference?

	Condition(Section& section, bool& state, SectionRouteTrack& track, Info&& info)
		: _state(state), correspondSection(section), track(track), info(info)
	{}

	operator bool() const
	{
		// ! to let it fit the if condition
		bool shouldExecute = false;
		if (_state) {
		} else if (correspondSection.allBranchDone()) {
		} else {
			shouldExecute = !_selfDone;
		}

		if (shouldExecute)
		{
			// still exist some problem
			if (track.include(info))
			{
				// set current
			} else {
				track.pushBack(info);
				// set the current position
			}
			
		}
		
		return shouldExecute;
	}

	~Condition()
	{
		if (*this) {
			// means it's leaf
			if (correspondSection._subSections.empty()) {
				correspondSection.markDone();
				_state = true;
			} 
			// else if (correspondSection._subSections.allBranchDone()) {
			// 	correspondSection.markDone();
			// 	_state = true;
			// }
		}


		// TODO here is key point
		track.popBack();
	}
};

inline Section::Section(Condition &condition)
{
	// register sub-section
	condition.correspondSection._subSections.emplace_back(this);

}

static vector<TESTCASE_FUNCTION_TYPE> _tests{};

class RegisterTestCase {
public:
	RegisterTestCase(TESTCASE_FUNCTION_TYPE testCase) // the const maybe not right
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
		shouldExecuteurn _fileName;
	}

	int16_t line() const
	{
		shouldExecuteurn _line;
	}
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
				t(Condition(testFunc, onceState, currentSectionTrack), onceState, currentSectionTrack); 
			} catch (AssertionFailure& f) {
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

// TODO add Info to this condition
#define TESTCASE(DESCRIPTION) static RegisterTestCase CAT(testcase, __LINE__) = (TESTCASE_FUNCTION_TYPE)[] (Condition condition, bool& onceState, SectionRouteTrack& track)

#define SECTION(DESCRIPTION) static Section CAT(section, __LINE__) { condition }; if (Condition condition{ CAT(section, __LINE__) , onceState, track, Info(__FILE__, __LINE__, DESCRIPTION) })
// below __FILE__ and __LINE__ maybe used wrong

#define ASSERT(EXP) do { if (!(EXP)) { throw AssertionFailure(#EXP, __FILE__, __LINE__ ); } } while(0)

// how to shouldExecutehrow this exption?
#define ASSERT_THROW(TYPE, EXP) 	\
	do {                        	\
    	try {                   	\
			(EXP);              	\
    	} catch (TYPE e) { }    	\
		  catch (...) { throw; }	\
		throw;						\
	} while(0)





