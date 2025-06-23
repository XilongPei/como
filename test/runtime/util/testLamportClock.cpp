//=========================================================================
// Copyright (C) 2024 The C++ Component Model(COMO) Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=========================================================================

#include <comosp.h>
#include <comoobj.h>
#include <gtest/gtest.h>
#include <int64.h>
#include <lamport_clock.h>

using namespace como;

TEST(LamportClock_Test, InitWithZero) {
    LamportClock clock;
    ASSERT_EQ(clock.get_time(), 0);
}

TEST(LamportClock_Test, LocalEvent) {
    LamportClock clock;
    clock.local_event();
    ASSERT_EQ(clock.get_time(), 1);
}

TEST(LamportClock_Test, SendEvent) {
    LamportClock clock;
    clock.send_event();
    ASSERT_EQ(clock.get_time(), 1);
}

TEST(LamportClock_Test, ReceiveEvent) {
    LamportClock clock;

    clock.receive_event(3);
    ASSERT_EQ(clock.get_time(), 4);

    clock.receive_event(2);
    ASSERT_EQ(clock.get_time(), 4);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
