#include "status.hpp"

#include <cassert>

using namespace scope::foundation;

int main()
{
    Status status = Status::Success;

    assert(status == Status::Success);

    return 0;
}
