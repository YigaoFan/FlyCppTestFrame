#pragma once

#include <functional>
#include <vector>
#include <string>
#include <tuple>
#include <iostream>
#include <utility>
#include <cstring>

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
using TESTCASE_FUNCTION_TYPE = function<void(Condition&&, bool&, SectionRouteTrack&, uint16_t&)>;
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
			<< sectionInfo.fileName << ":"
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
		return !_selfDone;
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
public:
	Section& correspondSection;
	SectionRouteTrack& track;

	Condition(Section& section, bool& state, SectionRouteTrack& track)
		: _state(state),
		_shouldExecute(true),
		correspondSection(section),
		track(track)
	{}

	operator bool()
	{
		// _state is true, means this run end
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
			if (!_state) {
				correspondSection.markDone();
				_state = true;
			}

			// save for remember:
//			// means it's leaf
//			if (correspondSection._subSections.empty()) {
//				correspondSection.markDone();
//				_state = true;
//			} else if (!uncaughtException()) { // not a leaf situation
//				if (!_state) {
//					correspondSection.markDone();
//					_state = true;
//				}
//			}

			if (!uncaughtException()) {
				track.moveBack();
			}
		}
	}

private:
	bool uncaughtException() const
	{
#if __cplusplus >= 201703L
		return std::uncaught_exceptions() > 0;
#elif __cplusplus >= 201103L
		return std::uncaught_exception();
#else
#error This program requires C++ version at least C++11, please update your compiler setting
#endif
	}
};

inline Section::Section(Condition &condition, Info info)
: _info(std::move(info))
{
	// register sub-section
	condition.correspondSection._subSections.emplace_back(this);
}

static vector<pair<Info, TESTCASE_FUNCTION_TYPE>> _tests_{};

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
		_tests_.emplace_back(make_pair(combination.first(), combination.second()));
	}
};

class AssertionFailure : public std::exception {
private:
	string _failureInfo;
public:
	AssertionFailure(const string &fileName, int16_t line, const string &description, const string &expression)
		: _failureInfo(fileName + ":" + std::to_string(line) + " " + description + "\n" + expression)
	{}

	const char* what() const noexcept override
	{
		return _failureInfo.c_str();
	}
};

static
void
allTest()
{
	auto& out = cout;
	auto log = [] (const string& exceptionTypeName, const string& exceptionContent, SectionRouteTrack& track) {
		out
			<< "\n"
			<< "Caught exception of type " << '\'' << exceptionTypeName << '\'' << endl
			<< exceptionContent << endl
			<< "Testcase state:" << endl
			;
		track.log(0, out);
	};

	for (auto& t : _tests_) {
		Section testCase(t.first);
		uint16_t successCount = 0;
		uint16_t failureCount = 0;
		out << "Testcase: " << testCase.info().description << endl;

		while (testCase.shouldExecute()) {
			auto onceState = false;
			SectionRouteTrack sectionTrack;
			sectionTrack.pushBack(testCase.info());

			try {
				t.second(std::move(Condition (testCase, onceState, sectionTrack)), onceState, sectionTrack, successCount);
			} catch (std::exception& e) {
				++ failureCount;
				log(string{typeid(e).name()}, string{e.what()}, sectionTrack);
			} catch (int i) {
				++ failureCount;
				log("int", std::to_string(i), sectionTrack);
			} catch (...) {
				++ failureCount;
				log(DEPEND_ON_COMPILER_SUPPORT(__cxxabiv1::__cxa_current_exception_type()->name(), "Unknown type"),
					"",
					sectionTrack);
			}
			if (!onceState) { // meas onceState not be changed
				break;
			}
		}

		out
			<< '\n'
			<< "Result: "
			<< failureCount << " Failed, "
			<< successCount << " Passed\n" << endl;
	}
}

static
const char *
justFileName(const char * str)
{
	// TODO need to test Windows compatibility
	return std::strrchr(str, '/') + 1;
}

#define PRIMITIVE_CAT(A, B) A##B
#define CAT(A, B) PRIMITIVE_CAT(A, B)

#define TESTCASE(DESCRIPTION) 																												\
	void CAT(testcase, __LINE__) (Condition&& , bool& , SectionRouteTrack&, uint16_t&);																	\
 	static RegisterTestCase CAT(registerTestcase, __LINE__) = Combination{Info(justFileName(__FILE__), __LINE__, DESCRIPTION)} = CAT(testcase, __LINE__); \
	void CAT(testcase, __LINE__) (Condition&& condition, bool& onceState, SectionRouteTrack& track, uint16_t& successCount)

#define SECTION(DESCRIPTION) static Section CAT(section, __LINE__) { condition, Info(justFileName(__FILE__), __LINE__, DESCRIPTION)};	\
	if (Condition condition{ CAT(section, __LINE__) , onceState, track})

#define ASSERT(EXP) 																			\
	do { 																						\
		if (!(EXP)) { 																			\
			throw AssertionFailure(justFileName(__FILE__), __LINE__, "ASSERTION FAILED", #EXP);	\
		} 																						\
		++successCount;																			\
	} while(0)

#define ASSERT_THROW(TYPE, EXP) 																		\
	do {                        																		\
    	try {																							\
			(void)(EXP);																				\
			throw AssertionFailure(justFileName(__FILE__), __LINE__, "No exception caught in", #EXP);	\
    	} catch (TYPE& e) { }																			\
		  catch (AssertionFailure& e) { throw e; }														\
		  catch (...) {																					\
    		throw AssertionFailure(																		\
    			justFileName(__FILE__), 																\
    			__LINE__, 																				\
    			string{"Catch a exception but not meet the required type: "} + #TYPE, 					\
    			#EXP);																					\
    	}																								\
		++successCount;																			\
	} while(0)
