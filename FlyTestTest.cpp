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
//	std::vector<int> a;
//	a.push_back(1);

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
//	}

//	SECTION("c") {
//		a.push_back(3);
//		std::cout << "c:" << std::endl;
//		showVector(a);
//	}
//};

TESTCASE("B") {

};
