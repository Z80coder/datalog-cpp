#include "catch.hpp"
#include "Variable.h"

using namespace datalog;

bool freeVariableTest() {
    Variable<int> intVar;
    return !intVar.isBound();
}

bool boundVariableTest() {
    Variable<int> intVar;
    intVar.bind(0);
    return intVar.isBound();
}

TEST_CASE( "An new variable is unbound", "[variable]" ) {
    REQUIRE( freeVariableTest() == true );
}

TEST_CASE( "A variable with a value is bound", "[variable]" ) {
    REQUIRE( boundVariableTest() == true );
}
