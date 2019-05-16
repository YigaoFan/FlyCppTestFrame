#include <utility>

#pragma once

#include <functional>
#include <vector>
#include <string>
#include <tuple>
#include <iostream>
#include <utility>

using std::pair;
using std::make_pair;
using std::string;
using std::vector;
using std::tuple;
using std::make_tuple;
using std::cout;
using std::endl;
using std::ostream;
using std::flush;
using std::function;

class SectionRouteTrack;
class Condition;
using TESTCASE_FUNCTION_TYPE = function<void(Condition, bool&, SectionRouteTrack&)>;
struct Info {
public:
	string fileName;
	int16_t line{};
	string description;

	Info() = default;

	Info(tuple<string, int16_t, string> t)
		: fileName(std::get<0>(t)), line(std::get<1>(t)), description(std::get<2>(t))
	{}

	Info(string fileName, int16_t line, string description)
		: fileName(std::move(fileName)), line(line), description(std::move(description))
	{}
};

class SectionRouteTrack {
private:
	vector<Info> _previousRoutePart{};
	Info _currentSection;
	bool _currentSectionValid{ false };
public:
	SectionRouteTrack() = default;

	SectionRouteTrack& pushBack(Info info)
	{
		if (_currentSectionValid) {
			_previousRoutePart.emplace_back(_currentSection);
		}
		_currentSection = std::move(info);
		_currentSectionValid = true;

		return *this;
	}

	void popBack()
	{
		_currentSection = std::move(_previousRoutePart.back());
		_previousRoutePart.pop_back();
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

	void reset()
	{
		_previousRoutePart.clear();
		_currentSectionValid = false;
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
			<< sectionInfo.fileName << ": "
			<< sectionInfo.line << ": "
			<< sectionInfo.description;
		}
		out.flush();
		return out;
	}
};

class Section {
	// Section don't have the data detail rely on Condition class, but Condition do.
	// So we can pass a condition as parameter in TESTCASE not reference without error
	friend class Condition;
public:
	Section(Condition& condition, Info info);
	explicit Section(Info info) : _info(std::move(info)) {}

	bool shouldExecute()
	{
		if (!_selfDone) {
			return true;
		} else if (!allBranchDone()){
			return true;
		}

		return false;
	}

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

	Info info() const
	{
		return _info;
	}

private:
	bool _selfDone{ false };
	vector<Section*> _subSections{};
	Info _info;
};

class Condition {
private:
	bool& _state;
	bool _shouldExecute;
	bool _allBranchDoneBeforeExecute;
public:
	Section& correspondSection;
	SectionRouteTrack& track;

	Condition(Section& section, bool& state, SectionRouteTrack& track)
		: _state(state), _shouldExecute(true), _allBranchDoneBeforeExecute(section.allBranchDone()), correspondSection(section), track(track)
	{}

	operator bool()
	{
		_shouldExecute = _state ? false : correspondSection.shouldExecute();

		if (_shouldExecute)
		{
			track.pushBack(correspondSection.info());
		}
		
		return _shouldExecute;
	}

	~Condition()
	{
		if (_shouldExecute) {
			// means it's leaf
			if (correspondSection._subSections.empty()) {
				correspondSection.markDone();
				_state = true;
			}
			if (!correspondSection._selfDone && _allBranchDoneBeforeExecute) {
				correspondSection.markDone();
			}
		}

		// TODO here is key point
//		track.popBack();
	}
};

inline Section::Section(Condition &condition, Info info)
: _info(std::move(info))
{
	// register sub-section
	condition.correspondSection._subSections.emplace_back(this);
}

static vector<pair<Info, TESTCASE_FUNCTION_TYPE>> _tests{};

class Combination {
private:
	pair<Info, TESTCASE_FUNCTION_TYPE> combination;
public:

	explicit Combination(Info info)
	{
		combination.first = std::move(info);
	}

	Combination& operator=(TESTCASE_FUNCTION_TYPE testCase)
	{
		combination.second = std::move(testCase);
		return *this;
	}

	Info first() const
	{
		return combination.first;
	}

	TESTCASE_FUNCTION_TYPE second() const
	{
		return combination.second;
	}
};

class RegisterTestCase {
public:
	RegisterTestCase(const Combination& combination)
	{
		_tests.emplace_back(make_pair(combination.first(), combination.second()));
	}
};

class AssertionFailure {
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

static
void
allTest()
{
	for (auto& t : _tests) {
		Section testFunc(t.first);
		while (!testFunc.allBranchDone()) { // t is false means t is not complete
			auto onceState = false;
			SectionRouteTrack sectionTrack;
			sectionTrack.pushBack(testFunc.info());
			try {
				t.second(Condition(testFunc, onceState, sectionTrack), onceState, sectionTrack);
			} catch (AssertionFailure& f) {
				// TODO show failure info
			} catch (...) {
				// TODO report uncaught exception
			}
		}
	}
}
// 我希望一个 TESTCASE 有两个状态，内部有个本次执行完毕的状态，外部有个标识所有 branch 执行完的的状态
// testFunc.allBranchDone() external state
// condition internal state
#define PRIMITIVE_CAT(A, B) A##B
#define CAT(A, B) PRIMITIVE_CAT(A, B)

#define TESTCASE(DESCRIPTION) static RegisterTestCase CAT(testcase, __LINE__) = Combination{Info(__FILE__, __LINE__, DESCRIPTION)} = (TESTCASE_FUNCTION_TYPE)[] (Condition condition, bool& onceState, SectionRouteTrack& track)

// Info should belong to Section
#define SECTION(DESCRIPTION) static Section CAT(section, __LINE__) { condition, Info(__FILE__, __LINE__, DESCRIPTION)}; if (Condition condition{ CAT(section, __LINE__) , onceState, track})
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





