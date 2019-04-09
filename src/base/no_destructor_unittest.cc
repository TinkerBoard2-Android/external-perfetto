
/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "perfetto/base/logging.h"
#include "perfetto/base/no_destructor.h"

namespace perfetto {
namespace base {
namespace {

class FatalDestructor {
 public:
  __attribute__((noreturn)) ~FatalDestructor() {
    PERFETTO_FATAL("Should not be called.");
  }
};

TEST(NoDestructorTest, DoesNotDestruct) {
  // Fatal without wrapper.
  EXPECT_DEATH({ FatalDestructor f; }, "");

  // Not destructed when wrapped.
  EXPECT_NO_FATAL_FAILURE({ NoDestructor<FatalDestructor> f; });
}

#pragma GCC diagnostic push
#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif
TEST(NoDestructorTest, DoesNotDestructStatic) {
  // Fatal without wrapper.
  ASSERT_DEATH(
      {
        static FatalDestructor f;
        exit(0);
      },
      "");

  // Not destructed when wrapped.
  ASSERT_EXIT(
      {
        static NoDestructor<FatalDestructor> f;
        exit(0);
      },
      ::testing::ExitedWithCode(0), "");
}
#pragma GCC diagnostic pop

class NonTrivial {
 public:
  NonTrivial(std::vector<int> v, std::unique_ptr<int> p)
      : vec_(v), ptr_(std::move(p)) {}

  std::vector<int> vec_;
  std::unique_ptr<int> ptr_;
};

TEST(NoDestructorTest, ContainedObjectUsable) {
  static NoDestructor<NonTrivial> x(std::vector<int>{1, 2, 3},
                                    std::unique_ptr<int>(new int(42)));

  ASSERT_THAT(x.ref().vec_, ::testing::ElementsAre(1, 2, 3));
  ASSERT_EQ(*x.ref().ptr_, 42);

  x.ref().vec_ = {4, 5, 6};
  ASSERT_THAT(x.ref().vec_, ::testing::ElementsAre(4, 5, 6));
}

}  // namespace
}  // namespace base
}  // namespace perfetto