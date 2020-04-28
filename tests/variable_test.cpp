#include "catch.hpp"
#include "variable.h"

using namespace datalog;

bool freeVariableTest()
{
    Variable<int> intVar;
    return !intVar.isBound();
}

bool boundVariableTest()
{
    Variable<int> intVar;
    intVar.bind(0);
    return intVar.isBound();
}

bool bindUnbindTest()
{
    Variable<int> intVar;
    intVar.bind(0);
    intVar.unbind();
    return !intVar.isBound();
}

bool storesValueTest()
{
    Variable<int> intVar;
    const int value = 100;
    intVar.bind(value);
    return intVar.isBound() and intVar.value() == value;
}

bool absentValueTest()
{
    Variable<int> intVar;
    const auto &val = intVar.value();
    return true;
}

TEST_CASE("variable binding", "[variable]")
{
    REQUIRE(freeVariableTest());
    REQUIRE(boundVariableTest());
    REQUIRE(bindUnbindTest());
    REQUIRE(bindUnbindTest());
    REQUIRE_THROWS_AS(absentValueTest(), std::bad_optional_access);
}
