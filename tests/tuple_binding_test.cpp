#include "catch.hpp"
#include "Datalog.h"

using namespace datalog;
using namespace std;

bool unbindTest()
{
    auto v = var<int>();
    v->bind(3);
    tuple<decltype(v), int> t{v, 3};
    v->unbind();
    bool returnVal = !get<0>(t)->isBound();
    deleteVar(v);
    // add a comment to force test CodeQL analysis
    return returnVal;
}

bool leakTest() {
    double* leak = new double[10];
    std::cout << "Hello!" << std::endl;
    delete[] leak;
    return true;
}

TEST_CASE("tuple binding test", "[tuple-binding]")
{
    REQUIRE(unbindTest());
    REQUIRE(leakTest());
}
