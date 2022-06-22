#include "simple_vector.h"
#include <iostream>
#include "simple_vector.h"

// Tests
#include "tests.h"

using namespace std;


int main() {
    TestTemporaryObjConstructor();
    TestTemporaryObjOperator();
    TestNamedMoveConstructor();
    TestNamedMoveOperator();
    TestNoncopiableMoveConstructor();
    TestNoncopiablePushBack();
    TestNoncopiableInsert();
    TestNoncopiableErase();
    Test1();
    Test2();
    TestReserveConstructor();
    TestReserveMethod();
    return 0;
}
