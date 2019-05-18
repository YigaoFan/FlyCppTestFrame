#pragma once

#include <functional>
#include <vector>
#include <string>
#include <tuple>
#include <iostream>
#include <utility>

// for unknown exception handle
#ifdef __clang__
#include "cxxabi.h"
#define DEPEND_ON_COMPILER_SUPPORT(SUPPORT_DO, NOT_SUPPORT_DO) SUPPORT_DO
#elif __GNUC__
#include "cxxabi.h"
#define DEPEND_ON_COMPILEER_SUPPORT(SUPPORT_DO, NOT_SUPPORT_DO) SUPPORT_DO
#else
#define DEPEND_ON_COMPILER_SUPPORT(SUPPORT_DO, NOT_SUPPORT_DO) NOT_SUPPORT_DO
#endif

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
using std::runtime_error;

class SectionRouteTrack;
class Condition;
using TESTCASE_FUNCTION_TYPE = function<void(Condition, bool&, SectionRouteTrack&)>;
struct Info {
public:
	string fileName;
	int16_t line{};
	string description;

	Info() = default;

	Info(string fileName, int16_t line, string description)
		: fileName(std::move(fileName)), line(line), description(std::move(description))
	{}
};

class SectionRouteTrack {
private:
	vector<Info> _route{};
	int16_t _currentSectionIndex = -1;
//	bool _currentSectionValid{ false };
public:
	SectionRouteTrack() = default;

	SectionRouteTrack& pushBack(Info info)
	{
		_route.emplace_back(std::move(info));
		_currentSectionIndex = _route.size() - 1;

		return *this;
	}

	void moveBack()
	{
		--_currentSectionIndex;
	}

	void log(int8_t initialIndent = 0, ostream& out = cout) const
	{
		if (_currentSectionIndex < 0) {
			throw runtime_error("Wrong invoke log, only need to call log when SECTION doesn't exit normally");
		}

		for (auto i = 0; i < _route.size(); ++i) {
			showNSpace(initialIndent, out);
			if (i == _currentSectionIndex) {
					out << "-> ";
			}
			showSectionInfo(_route[i], out) << endl;
			++initialIndent;
		}
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

	static ostream& showSectionInfo(const Info& sectionInfo, ostream& out)
	{
		out
			<< sectionInfo.fileName << ": "
			<< sectionInfo.line << ": "
			<< sectionInfo.description;

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

			if constexpr (__cplusplus == 201703L) {
				if (std::uncaught_exceptions()) {
					// stop move mark
				} else {
					track.moveBack();
				}
			} else if constexpr (__cplusplus >= 201103L) {
				if (std::uncaught_exception()) {
					// stop move mark
				} else {
					track.moveBack();
				}
			} else {
				// TODO meta-program?
				// could use macro
				throw
					runtime_error
						("This program require C++ version at least C++11, please update your compiler setting");
			}

		}
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

class AssertionFailure : std::exception {
private:
	string failureInfo;
public:
	AssertionFailure(const string &fileName, int16_t line, const string &description, const string &expression)
		: failureInfo(fileName + ":" + std::to_string(line) + description + '\n' + expression)
	{}

	const char* what() const noexcept override
	{
		return failureInfo.c_str();
	}
};

static
void
allTest()
{
	auto log = [] (const string& exceptionTypeName, const string& exceptionContent, SectionRouteTrack& track) {
		cout
			<< "Caught exception of type " << '\'' << exceptionTypeName << '\'' << endl
			<< exceptionContent << endl
			<< "Testcase state:" << endl
			;
		track.log();
	};

	for (auto& t : _tests) {
		Section testFunc(t.first);
		while (!testFunc.allBranchDone()) { // t is false means t is not complete
			auto onceState = false;
			SectionRouteTrack sectionTrack;
			sectionTrack.pushBack(testFunc.info());
			try {
				t.second(Condition(testFunc, onceState, sectionTrack), onceState, sectionTrack);
			} catch (std::exception& e) {
				log(typeid(e).name(), e.what(), sectionTrack);
			} catch (int i) {
				log("int", std::to_string(i), sectionTrack);
			} catch (...) {
				log(DEPEND_ON_COMPILER_SUPPORT(__cxxabiv1::__cxa_current_exception_type()->name(), "Unknown type"),
					"",
					sectionTrack);
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

#define SECTION(DESCRIPTION) static Section CAT(section, __LINE__) { condition, Info(__FILE__, __LINE__, DESCRIPTION)}; if (Condition condition{ CAT(section, __LINE__) , onceState, track})

#define ASSERT(EXP) 																\
	do { 																			\
		if (!(EXP)) { 																\
			throw AssertionFailure(__FILE__, __LINE__, "ASSERTION FAILED", #EXP);	\
		} 																			\
	} while(0)

#define ASSERT_THROW(TYPE, EXP) 														\
	do {                        														\
    	try {																			\
			(EXP);																		\
    	} catch (TYPE e) { }															\
		  catch (...) {																	\
    		throw AssertionFailure(														\
    			__FILE__, 																\
    			__LINE__, 																\
    			string{"Catch a exception but not meet the required type "} + #TYPE, 	\
    			#EXP);																	\
    	}																				\
		throw AssertionFailure(__FILE__, __LINE__, "No exception caught in", #EXP);		\
	} while(0)
