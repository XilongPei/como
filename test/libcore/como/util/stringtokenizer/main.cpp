//=========================================================================
// Copyright (C) 2018 The C++ Component Model(COMO) Open Source Project
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

#include "como.util.CStringTokenizer.h"
#include "como.util.IStringTokenizer.h"
#include <comosp.h>
#include <gtest/gtest.h>

using namespace como;
using como::util::CStringTokenizer;
using como::util::IStringTokenizer;
using como::util::IID_IStringTokenizer;

TEST(StringTokenizerTest, StringTokenizerTest)
{
    AutoPtr<IStringTokenizer> tokenizer;
    CStringTokenizer::New("hello world hello shanghai",
            " ", IID_IStringTokenizer, (IInterface**)&tokenizer);
    Integer count;
    tokenizer->CountTokens(count);
    EXPECT_EQ(4, count);
    String token1, token2, token3, token4;
    tokenizer->NextToken(token1);
    EXPECT_STREQ("hello", token1.string());
    tokenizer->NextToken(token2);
    EXPECT_STREQ("world", token2.string());
    tokenizer->NextToken(token3);
    tokenizer->NextToken(token4);
    EXPECT_STREQ("shanghai", token4.string());
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
