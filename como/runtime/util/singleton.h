//=========================================================================
// Copyright (C) 2022 The C++ Component Model(COMO) Open Source Project
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

#ifndef __SINGLETON_H__
#define __SINGLETON_H__

namespace como {

#define DECLARE_SINGLETON(class_name)                   \
protected:                                              \
    class_name();                                       \
    static class_name* m_pInstance;                     \
public:                                                 \
    static class_name* Instance();

#define IMPLEMENT_SINGLETON(class_name)                 \
    class_name* class_name::m_pInstance = nullptr;      \
    class_name* class_name::Instance()                  \
    {                                                   \
        if (nullptr == m_pInstance)                     \
            m_pInstance = new class_name();             \
        return m_pInstance;                             \
    }

} // namespace como

#endif // __SINGLETON_H__
