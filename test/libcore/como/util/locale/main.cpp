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

#include "como/util/LocaleFactory.h"
#include "como.util.CLocale.h"
#include "como.util.ILocale.h"
#include <comosp.h>
#include <gtest/gtest.h>

using namespace como;
using como::util::CLocale;
using como::util::ILocale;
using como::util::IID_ILocale;
using como::util::LocaleFactory;

TEST(LocaleTest, NewTest)
{
    AutoPtr<ILocale> locale;
    ECode ec = CLocale::New(String("en"), IID_ILocale, (IInterface**)&locale);
    EXPECT_EQ(ec, NOERROR);
    EXPECT_TRUE(locale != nullptr);
    String country;
    locale->GetCountry(country);
    EXPECT_STREQ(country.string(), "");
}

TEST(LocaleTest, GetDefaultTest)
{
    AutoPtr<ILocale> locale;
    LocaleFactory::GetDefault(&locale);
    EXPECT_TRUE(locale != nullptr);
    String language, country;
    locale->GetLanguage(language);
    EXPECT_STREQ(language.string(), "en");
    locale->GetCountry(country);
    EXPECT_STREQ(country.string(), "US");
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
