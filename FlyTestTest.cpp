#include <stdexcept>
#include "FlyTest.hpp"

bool is_equal(int a, int b)
{
    return (a == b);
}

TESTCASE("My First TestCase")
{
    SECTION("No problem")
    {
        ASSERT(true);
        ASSERT(is_equal(1, 1));
        ASSERT_THROW(int, throw 1);
    }

    SECTION("Assertion failed")
    {
        SECTION("a")
        {
            ASSERT(false);
        }

        SECTION("b")
        {
            ASSERT(is_equal(1, 2));
        }

        SECTION("c")
        {
        }

        ASSERT_THROW(int, 1);
    }

    SECTION("Exceptions")
    {
        SECTION("a")
        {
            throw 1;
        }

        SECTION("b")
        {
            throw ::std::logic_error("stop here");
        }
    }

}

int main()
{
    allTest();
}
