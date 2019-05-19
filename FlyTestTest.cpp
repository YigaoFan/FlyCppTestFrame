#include <iostream>
#include "FlyTest.hpp"

static
void
showVector(const std::vector<int> v)
{
	for (const auto &item : v) {
		std::cout << item << std::endl;
	}
}

//TESTCASE("A") {
//	ASSERT(true);
//	ASSERT_THROW(int , throw 1);
//	std::vector<int> a;
//	a.push_back(1);
//
//	SECTION("a") {
//		a.push_back(2);
//		std::cout << "a:" << std::endl;
//		showVector(a);
//
//		SECTION("b") {
//			a.push_back(3);
//			std::cout << "b:" << std::endl;
//			showVector(a);
//		}
//
//		SECTION("d") {
//			a.pop_back();
//			std::cout << "d:" << std::endl;
//			showVector(a);
//		}
//
//		throw 1;
//	}
//
//	SECTION("c") {
//		a.push_back(3);
//		std::cout << "c:" << std::endl;
//		showVector(a);
//	}
//}
//
//TESTCASE("B") {
//	ASSERT_THROW(unsigned , throw 1);
//	cout << "GET HERE" << endl;
//}

bool
is_equal(int a, int b) {
	return (a == b);
}

TESTCASE("My First TestCase") {

	SECTION("No problem") {
		ASSERT(true);
		ASSERT(is_equal(1,1));
		ASSERT_THROW(int, throw 1);
	}

	SECTION("Assertion failed") {
		SECTION("a") {
			ASSERT(false);
		}

		SECTION("b") {
			ASSERT(is_equal(1,2));
		}

		SECTION("c") {
		}

		ASSERT_THROW(int, 1);
	}

	SECTION("Exceptions") {
		SECTION("a") {
			throw 1;
		}

		SECTION("b") {
			throw ::std::logic_error("stop here");
		}
	}

}

TESTCASE("Hello") {

}