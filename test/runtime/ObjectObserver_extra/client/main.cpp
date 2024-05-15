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

#include <stdio.h>
#include <gtest/gtest.h>
#include <comoapi.h>
#include <comoobj.h>
#include "reflection/CMetaMethod.h"
#include "ObjectObserverTestUnit2.h"

TEST(testObjectObserverTest, TesttestObjectObserver)
{
    AutoPtr<IMetaComponent> mc;
    CoGetComponentMetadata(CID_ObjectObserverTestUnit2, nullptr, mc);
    AutoPtr<IMetaCoclass> klass;
    mc->GetCoclass("como::test::reflection::CMethodTester2", klass);
    AutoPtr<IInterface> obj;
    klass->CreateObject(IID_IInterface, &obj);

    IObjectObserver* observer = IObjectObserver::Probe(obj);
    EXPECT_NE(nullptr, observer);
    IObject::Probe(obj)->SetObjectObserver(observer);

    /**
     * You can't monitor the first reference of `obj` because observer doesn't
     * exist yet.
     */
    IObject::Probe(obj)->AddRef();  // ==== call OnFirstRef() ====
    IObject::Probe(obj)->Release(); // ==== call OnLastStrongRef() ====

    como::Object *objTmp = Object::From(IObject::Probe(obj));
    objTmp->TrackMe(true, true);
    objTmp->PrintRefs("Obj_1");

    AutoPtr<IMetaMethod> method;
    klass->GetMethod("TestMethod2", "(FF&)E", method);

#if DEBUG_REFS_LightRefBase
    CMetaMethod *cmethod = CMetaMethod::From(method);
    cmethod->TrackMe(true, true);
    cmethod->PrintRefs("Obj_2");
#endif

    AutoPtr<IArgumentList> args;
    method->CreateArgumentList(args);
    Float arg = 9.9, result;
    args->SetInputArgumentOfFloat(0, arg);
    args->SetOutputArgumentOfFloat(1, reinterpret_cast<HANDLE>(&result));
    method->Invoke(obj, args);
    IObject::Probe(obj)->GetCoclass(klass);
    EXPECT_FLOAT_EQ(arg, result);
    String name, ns;
    klass->GetName(name);
    klass->GetNamespace(ns);
    EXPECT_STREQ("como::test::reflection", ns.string());
    EXPECT_STREQ("CMethodTester2", name.string());
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
