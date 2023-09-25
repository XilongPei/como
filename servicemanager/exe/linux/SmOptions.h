//=========================================================================
// Copyright (C) 2023 The C++ Component Model(COMO) Open Source Project
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

#ifndef __JING_OPTIONS_H__
#define __JING_OPTIONS_H__

#include <comoapi.h>
#include <comosp.h>
#include <comoref.h>

namespace jing {

class SmOptions
{
public:
    inline SmOptions(
        /* [in] */ int argc,
        /* [in] */ char** argv);

    inline bool DoShowUsage() const;
    inline String GetPaxosServer() const;

    bool HasErrors() const;

    void ShowErrors() const;

    void ShowUsage() const;

private:
    void Parse(
        /* [in] */ int argc,
        /* [in] */ char** argv);

private:
    String mProgram;
    String mIllegalOptions;
    String mPaxosServer;

    bool mShowUsage = false;
    bool mShowVersion = false;
};

SmOptions::SmOptions(
    /* [in] */ int argc,
    /* [in] */ char** argv)
    : mPaxosServer(nullptr)
{
    Parse(argc, argv);
}

bool SmOptions::DoShowUsage() const
{
    return mShowUsage;
}

String SmOptions::GetPaxosServer() const
{
    return mPaxosServer;
}

} // namespace jing

#endif // __JING_OPTIONS_H__
