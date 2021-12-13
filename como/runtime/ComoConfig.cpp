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

#include "ComoConfig.h"

namespace como {

int ComoConfig::ThreadPool_MAX_THREAD_NUM = 2;
int ComoConfig::ThreadPoolChannelInvoke_MAX_THREAD_NUM = 2;
CpuInvokeDsa ComoConfig::cpuInvokeDsa[MAX_DSA_IN_ONE_SYSTEM] = {nullptr};

/*
 * /como/como/como/runtime/util/comolog.h
 * int Logger::sLevel = DEBUG;
 */
int Logger_sLevel = 0;

// ns, 30s
Long ComoConfig::TPCI_TASK_EXPIRES = 1000000000L * 30;

} // namespace como
