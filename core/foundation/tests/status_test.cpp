/**
 * @file status_test.cpp
 * @brief Unit tests for the Foundation Status type.
 */

#include "status.hpp"

#include <cassert>

using scope::foundation::Status;

int main()
{
    Status status = Status::Success;

    assert(status == Status::Success);

    return 0;
}
