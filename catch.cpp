TEST_CASE("A") {
	// do A_something
	SECTION("A_1") {
		// do A_1_something
		SECTION("A_1_1") {
			// do A_1_1_something
		}
	}
}

// default added item has a static bool state
// static 的 test_list 可以在函数外进行下面这个操作吗：不可以
// 那就利用类的构造函数来进行一些类似以下的这个操作
test_list += auto TEST_CASE_A() {
	// do A_something
	static Section section_A_1(condition); // register a son in father Section by the condition reference. But here has a problem: because where to find the top level condition?
	if (Condition condition = section_A_1) {
		// do A_1_something
		static Section section_A_1_1(condition);
		if (Condition condition = section_A_1_1) {

		}
		condition.end(); // how to implement this statement
	}
}

// The class Condition has some data reference to the section object
class Condition {
	~Condtion()
	{
		// if condition(or the corresponding Section has son, no special work to do)
		// else has no sons, need mark "tested" from the bottom of the tree to the top
	}
}
