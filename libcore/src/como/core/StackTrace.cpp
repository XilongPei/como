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

#include "como/core/AutoLock.h"
#include "como/core/CStackTraceElement.h"
#include "como/core/NativeBacktrace.h"
#include "como/core/StackTrace.h"
#include "como.core.IStackTraceElement.h"

using como::io::IID_ISerializable;

namespace como {
namespace core {

COMO_INTERFACE_IMPL_2(StackTrace, SyncObject, IStackTrace, ISerializable);

ECode StackTrace::Constructor()
{
    FillInStackTrace();
    return NOERROR;
}

ECode StackTrace::Constructor(
    /* [in] */ const String& message)
{
    FillInStackTrace();
    mDetailMessage = message;
    return NOERROR;
}

ECode StackTrace::PrintStackTrace()
{
    return NOERROR;
}

ECode StackTrace::PrintStackTrace(
    /* [in] */ IPrintStream* s)
{
    AutoPtr<PrintStreamOrWriter> ps = new WrappedPrintStream(s);
    return PrintStackTrace(ps);
}

ECode StackTrace::PrintStackTrace(
    /* [in] */ PrintStreamOrWriter* s)
{
    AutoLock lock(s->Lock());

    Array<IStackTraceElement*> trace = GetOurStackTrace();
    for (Integer i = 0; i < trace.GetLength(); i++) {
        IStackTraceElement* traceElement = trace[i];
        s->Println(traceElement);
    }
    return NOERROR;
}

ECode StackTrace::PrintStackTrace(
    /* [in] */ IPrintWriter* s)
{
    AutoPtr<PrintStreamOrWriter> ps = new WrappedPrintWriter(s);
    return PrintStackTrace(ps);
}

ECode StackTrace::FillInStackTrace()
{
    mFrameCount = GetBacktrace(mFrames.GetPayload(), mFrameSps.GetPayload(),
                                                            mFrames.GetLength());
    return NOERROR;
}

ECode StackTrace::GetStackTrace(
    /* [out, callee] */ Array<IStackTraceElement*>* stack)
{
    VALIDATE_NOT_NULL(stack);

    *stack = GetOurStackTrace().Clone();
    return NOERROR;
}

Array<IStackTraceElement*> StackTrace::GetOurStackTrace()
{
    if (mFrameCount > 0) {
        Array<IStackTraceElement*> frameElements(mFrameCount);

        DumpBacktrace(mFrames.GetPayload(), mFrameSps.GetPayload(), mFrameCount,
                                                                    frameElements);
        VOLATILE_SET(mStackTrace, mFrameCount);
    }

    return mStackTrace;
}

} // namespace como
} // namespace core
