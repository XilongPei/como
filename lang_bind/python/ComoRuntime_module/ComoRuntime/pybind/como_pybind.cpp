//=========================================================================
// Copyright (C) 2021 The C++ Component Model(COMO) Open Source Project
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

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <comoapi.h>

using namespace pybind11::literals;
namespace py = pybind11;

using namespace como;

HANDLE myCoGetComponentMetadataWithPath(std::string s) {
    String path(s.c_str());
    AutoPtr<IMetaComponent> mc;
    CoGetComponentMetadataWithPath(path, nullptr, mc);
    return reinterpret_cast<HANDLE>((IMetaComponent *)mc);
}

PYBIND11_MODULE(como_pybind, m) {

    // functions in comoreflapi.h
    m.def(
        "CoGetComponentMetadataWithPath",
        [](std::string s) {
            HANDLE handle = myCoGetComponentMetadataWithPath(s);
            return handle;
        },
        pybind11::return_value_policy::reference);
}
