# FlyCppTestFrame
A implementation of a C++ test frame like Catch2 part usage

This frame should use like the code below. The good of this framework also Catch is that you can freely nest tests. As you know, there are usually some preconditions that need to be constructed during the test. Some times they are duplicate. That's very convenient and elegant. Then it requires C++ 11 and above.

```c++
#include <vector>
#include "FlyTest.hpp" // include this header file to use

TESTCASE("First TestCase") {
	// set up code
	std::vector<int> a{};
	a.push_back(1);

	// this SECTION will not affect the "Delete element" SECTION
	SECTION("Add element") {
		a.push_back(2);
		// ASSERT(EXP)
		ASSERT(a[0] == 1);
		ASSERT(a[1] == 2);

		// SECTION can be nested without limit
		SECTION("Change element") {
			a[0] = 3;
			ASSERT(a[0] == 3);
			ASSERT(a[1] == 2);
		}
	}

	SECTION("Delete element") {
		a.pop_back();
		ASSERT(a.empty());
	}

	// some tear down code
}

// then in your main function, you should call allTest()
int
main()
{
	allTest();

	return 0;
}
```

It also can assert throw exception, like:
```c++
// ASSERT_THROW(TYPE, EXP)
ASSERT_THROW(int, throw 1);
```



Material of the main idea How to implement this frame:

1. [How to implement SECTION in Catch](https://zhuanlan.zhihu.com/p/24535431)
2. [Answer some questions](https://zhuanlan.zhihu.com/p/24547564)
3. [Another implement](https://zhuanlan.zhihu.com/p/24552354)

---------
中文版说明：

这是一份可以使用 Catch2 部分用法的测试框架。

这个框架需要像下面的代码这样使用。这个框架好的地方在于你可以自由的嵌套测试代码，当然这也是 Catch 框架的优点。你知道的，在测试中通常是有些前置条件需要去构造。有时他们是重复。所以下面这样使用非常的方便和优雅。然后，这个框架对 C++ 版本的要求是至少是 C++11。

```c++
#include <vector>
#include "FlyTest.hpp" // 需要包含这个头文件

TESTCASE("First TestCase") {
	// set up code
	std::vector<int> a{};
	a.push_back(1);

	// 这个 SECTION 的代码不会影响下面的 "Delete element" SECTION
	SECTION("Add element") {
		a.push_back(2);
		// ASSERT(EXP)
		ASSERT(a[0] == 1);
		ASSERT(a[1] == 2);

		// SECTION can be nested without limit
		SECTION("Change element") {
			a[0] = 3;
			ASSERT(a[0] == 3);
			ASSERT(a[1] == 2);
		}
	}

	SECTION("Delete element") {
		a.pop_back();
		ASSERT(a.empty());
	}

	// some tear down code
}

// 然后你需要在你的 main 函数里调用 allTest()
int
main()
{
	allTest();

	return 0;
}
```

它也可以断言抛出的异常类型，就像下面这样:
```c++
// ASSERT_THROW(TYPE, EXP)
ASSERT_THROW(int, throw 1);
```


关于实现这个框架的主要方法的一些材料:

1. [如何实现 Catch 里的 SECTION](https://zhuanlan.zhihu.com/p/24535431)
2. [答疑](https://zhuanlan.zhihu.com/p/24547564)
3. [另一份类似用法的框架实现](https://zhuanlan.zhihu.com/p/24552354)
