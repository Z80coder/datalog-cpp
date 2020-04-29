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
    return returnVal;
}

TEST_CASE("tuple binding test", "[tuple-binding]")
{
    REQUIRE(unbindTest());
}
