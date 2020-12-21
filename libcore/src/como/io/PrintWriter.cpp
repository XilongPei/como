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
#include "como/core/CoreUtils.h"
#include "como/core/StringUtils.h"
#include "como/core/Thread.h"
#include "como/io/CBufferedWriter.h"
#include "como/io/CFile.h"
#include "como/io/CFileOutputStream.h"
#include "como/io/COutputStreamWriter.h"
#include "como/io/PrintWriter.h"
#include "como/io/charset/Charset.h"
#include "como/security/AccessController.h"
#include "como/security/action/CGetPropertyAction.h"
#include "como/util/CFormatter.h"
#include "como/util/Locale.h"
#include "como.core.IThread.h"
#include "como.security.IPrivilegedAction.h"
#include <comolog.h>

using como::core::AutoLock;
using como::core::CoreUtils;
using como::core::IThread;
using como::core::StringUtils;
using como::core::Thread;
using como::io::charset::Charset;
using como::security::AccessController;
using como::security::IPrivilegedAction;
using como::security::IID_IPrivilegedAction;
using como::security::action::CGetPropertyAction;
using como::util::CFormatter;
using como::util::IID_IFormatter;
using como::util::Locale;

namespace como {
namespace io {

COMO_INTERFACE_IMPL_1(PrintWriter, Writer, IPrintWriter);

ECode PrintWriter::ToCharset(
    /* [in] */ const String& csn,
    /* [out] */ AutoPtr<ICharset>& cs)
{
    if (csn.IsNull()) {
        Logger::E("PrintWriter", "charsetName is null");
        return como::core::E_NULL_POINTER_EXCEPTION;
    }
    ECode ec = Charset::ForName(csn, cs);
    return SUCCEEDED(ec) ? NOERROR : E_UNSUPPORTED_ENCODING_EXCEPTION;
}

ECode PrintWriter::Constructor(
    /* [in] */ IWriter* outstream)
{
    return Constructor(outstream, false);
}

ECode PrintWriter::Constructor(
    /* [in] */ IWriter* outstream,
    /* [in] */ Boolean autoFlush)
{
    Writer::Constructor(ISynchronize::Probe(outstream));
    mOut = outstream;
    mAutoFlush = autoFlush;
    AutoPtr<IPrivilegedAction> lsAction;
    CGetPropertyAction::New(String("line.separator"),
            IID_IPrivilegedAction, (IInterface**)&lsAction);
    AutoPtr<IInterface> lsRet;
    FAIL_RETURN(AccessController::DoPrivileged(lsAction, lsRet));
    mLineSeparator = CoreUtils::Unbox(ICharSequence::Probe(lsRet));
    return NOERROR;
}

ECode PrintWriter::Constructor(
    /* [in] */ IOutputStream* outstream)
{
    return Constructor(outstream, false);
}

ECode PrintWriter::Constructor(
    /* [in] */ IOutputStream* outstream,
    /* [in] */ Boolean autoFlush)
{
    AutoPtr<IWriter> osw, bw;
    COutputStreamWriter::New(outstream, IID_IWriter, (IInterface**)&osw);
    CBufferedWriter::New(osw, IID_IWriter, (IInterface**)&bw);
    FAIL_RETURN(Constructor(bw, autoFlush));

    // save print stream for error propagation
    if (IPrintStream::Probe(outstream) != nullptr) {
        mPsOut = IPrintStream::Probe(outstream);
    }
    return NOERROR;
}

ECode PrintWriter::Constructor(
    /* [in] */ const String& fileName)
{
    AutoPtr<IOutputStream> fos;
    CFileOutputStream::New(fileName, IID_IOutputStream, (IInterface**)&fos);
    AutoPtr<IWriter> osw, bw;
    COutputStreamWriter::New(fos, IID_IWriter, (IInterface**)&osw);
    CBufferedWriter::New(osw, IID_IWriter, (IInterface**)&bw);
    return Constructor(bw, false);
}

ECode PrintWriter::Constructor(
    /* [in] */ ICharset* charset,
    /* [in] */ IFile* file)
{
    AutoPtr<IOutputStream> fos;
    CFileOutputStream::New(file, IID_IOutputStream, (IInterface**)&fos);
    AutoPtr<IWriter> osw, bw;
    COutputStreamWriter::New(fos, charset, IID_IWriter, (IInterface**)&osw);
    CBufferedWriter::New(osw, IID_IWriter, (IInterface**)&bw);
    return Constructor(bw, false);
}

ECode PrintWriter::Constructor(
    /* [in] */ const String& fileName,
    /* [in] */ const String& csn)
{
    AutoPtr<IFile> f;
    CFile::New(fileName, IID_IFile, (IInterface**)&f);
    AutoPtr<ICharset> cs;
    FAIL_RETURN(ToCharset(csn, cs));
    return Constructor(cs, f);
}

ECode PrintWriter::Constructor(
    /* [in] */ IFile* file)
{
    AutoPtr<IOutputStream> fos;
    CFileOutputStream::New(file, IID_IOutputStream, (IInterface**)&fos);
    AutoPtr<IWriter> osw, bw;
    COutputStreamWriter::New(fos, IID_IWriter, (IInterface**)&osw);
    CBufferedWriter::New(osw, IID_IWriter, (IInterface**)&bw);
    return Constructor(bw, false);
}

ECode PrintWriter::Constructor(
    /* [in] */ IFile* file,
    /* [in] */ const String& csn)
{
    AutoPtr<ICharset> cs;
    FAIL_RETURN(ToCharset(csn, cs));
    return Constructor(cs, file);
}

ECode PrintWriter::EnsureOpen()
{
    if (mOut == nullptr) {
        Logger::E("PrintWriter", "Stream closed");
        return E_IO_EXCEPTION;
    }
    return NOERROR;
}

ECode PrintWriter::Flush()
{
    {
        AutoLock lock(mLock);
        ECode ec = EnsureOpen();
        if (FAILED(ec)) goto ERROR;
        ec = mOut->Flush();
        if (FAILED(ec)) goto ERROR;
        return NOERROR;
    }

ERROR:
    mTrouble = true;
    return NOERROR;
}

ECode PrintWriter::Close()
{
    {
        AutoLock lock(mLock);
        if (mOut == nullptr) {
            return NOERROR;
        }
        ECode ec = mOut->Close();
        if (FAILED(ec)) goto ERROR;
        mOut = nullptr;
        return NOERROR;
    }

ERROR:
    mTrouble = true;
    return NOERROR;
}

ECode PrintWriter::CheckError(
    /* [out] */ Boolean& hasErrors)
{
    if (mOut != nullptr) {
        Flush();
    }
    if (IPrintWriter::Probe(mOut) != nullptr) {
        return IPrintWriter::Probe(mOut)->CheckError(hasErrors);
    }
    else if (mPsOut != nullptr) {
        return mPsOut->CheckError(hasErrors);
    }
    hasErrors = mTrouble;
    return NOERROR;
}

void PrintWriter::SetError()
{
    mTrouble = true;
}

void PrintWriter::ClearError()
{
    mTrouble = false;
}

ECode PrintWriter::Write(
    /* [in] */ Integer c)
{
    ECode ec;
    {
        AutoLock lock(mLock);
        ec = EnsureOpen();
        if (FAILED(ec)) goto ERROR;
        ec = mOut->Write(c);
        if (FAILED(ec)) goto ERROR;
        return NOERROR;
    }

ERROR:
    if (ec == E_INTERRUPTED_IO_EXCEPTION) {
        AutoPtr<IThread> t;
        Thread::GetCurrentThread(&t);
        t->Interrupt();
    }
    else if (ec == E_IO_EXCEPTION) {
        mTrouble = true;
    }
    return NOERROR;
}

ECode PrintWriter::Write(
    /* [in] */ const Array<Char>& buffer,
    /* [in] */ Integer off,
    /* [in] */ Integer len)
{
    ECode ec;
    {
        AutoLock lock(mLock);
        ec = EnsureOpen();
        if (FAILED(ec)) goto ERROR;
        ec = mOut->Write(buffer, off, len);
        if (FAILED(ec)) goto ERROR;
        return NOERROR;
    }

ERROR:
    if (ec == E_INTERRUPTED_IO_EXCEPTION) {
        AutoPtr<IThread> t;
        Thread::GetCurrentThread(&t);
        t->Interrupt();
    }
    else if (ec == E_IO_EXCEPTION) {
        mTrouble = true;
    }
    return NOERROR;
}

ECode PrintWriter::Write(
    /* [in] */ const Array<Char>& buffer)
{
    return Write(buffer, 0, buffer.GetLength());
}

ECode PrintWriter::Write(
    /* [in] */ const String& str,
    /* [in] */ Integer off,
    /* [in] */ Integer len)
{
    ECode ec;
    {
        AutoLock lock(mLock);
        ec = EnsureOpen();
        if (FAILED(ec)) goto ERROR;
        ec = mOut->Write(str, off, len);
        if (FAILED(ec)) goto ERROR;
        return NOERROR;
    }

ERROR:
    if (ec == E_INTERRUPTED_IO_EXCEPTION) {
        AutoPtr<IThread> t;
        Thread::GetCurrentThread(&t);
        t->Interrupt();
    }
    else if (ec == E_IO_EXCEPTION) {
        mTrouble = true;
    }
    return NOERROR;
}

void PrintWriter::NewLine()
{
    ECode ec;
    {
        AutoLock lock(mLock);
        ec = EnsureOpen();
        if (FAILED(ec)) goto ERROR;
        ec = mOut->Write(mLineSeparator);
        if (FAILED(ec)) goto ERROR;
        if (mAutoFlush) {
            ec = mOut->Flush();
            if (FAILED(ec)) goto ERROR;
        }
        return;
    }

ERROR:
    if (ec == E_INTERRUPTED_IO_EXCEPTION) {
        AutoPtr<IThread> t;
        Thread::GetCurrentThread(&t);
        t->Interrupt();
    }
    else if (ec == E_IO_EXCEPTION) {
        mTrouble = true;
    }
}

ECode PrintWriter::Print(
    /* [in] */ Boolean b)
{
    return Write(b ? String("true") : String("false"));
}

ECode PrintWriter::PrintChar(
    /* [in] */ Char c)
{
    return Write(c);
}

ECode PrintWriter::Print(
    /* [in] */ Integer i)
{
    return Write(StringUtils::ToString(i));
}

ECode PrintWriter::Print(
    /* [in] */ Long l)
{
    return Write(StringUtils::ToString(l));
}

ECode PrintWriter::Print(
    /* [in] */ Float f)
{
    return Write(StringUtils::ToString(f));
}

ECode PrintWriter::Print(
    /* [in] */ Double d)
{
    return Write(StringUtils::ToString(d));
}

ECode PrintWriter::Print(
    /* [in] */ const Array<Char>& s)
{
    return Write(s);
}

ECode PrintWriter::Print(
    /* [in] */ const String& s)
{
    if (s.IsNull()) {
        return Write(String("null"));
    }
    return Write(s);
}

ECode PrintWriter::Print(
    /* [in] */ IInterface* obj)
{
    return Write(Object::ToString(obj));
}

ECode PrintWriter::Println()
{
    NewLine();
    return NOERROR;
}

ECode PrintWriter::Println(
    /* [in] */ Boolean b)
{
    AutoLock lock(mLock);
    Print(b);
    Println();
    return NOERROR;
}

ECode PrintWriter::PrintCharln(
    /* [in] */ Char c)
{
    AutoLock lock(mLock);
    PrintChar(c);
    Println();
    return NOERROR;
}

ECode PrintWriter::Println(
    /* [in] */ Integer i)
{
    AutoLock lock(mLock);
    Print(i);
    Println();
    return NOERROR;
}

ECode PrintWriter::Println(
    /* [in] */ Long l)
{
    AutoLock lock(mLock);
    Print(l);
    Println();
    return NOERROR;
}

ECode PrintWriter::Println(
    /* [in] */ Float f)
{
    AutoLock lock(mLock);
    Print(f);
    Println();
    return NOERROR;
}

ECode PrintWriter::Println(
    /* [in] */ Double d)
{
    AutoLock lock(mLock);
    Print(d);
    Println();
    return NOERROR;
}

ECode PrintWriter::Println(
    /* [in] */ const Array<Char>& s)
{
    AutoLock lock(mLock);
    Print(s);
    Println();
    return NOERROR;
}

ECode PrintWriter::Println(
    /* [in] */ const String& s)
{
    AutoLock lock(mLock);
    Print(s);
    Println();
    return NOERROR;
}

ECode PrintWriter::Println(
    /* [in] */ IInterface* obj)
{
    String s = Object::ToString(obj);
    AutoLock lock(mLock);
    Print(s);
    Println();
    return NOERROR;
}

ECode PrintWriter::Printf(
    /* [in] */ const String& format,
    /* [in] */ const Array<IInterface*>* args)
{
    return Format(format, args);
}

ECode PrintWriter::Printf(
    /* [in] */ ILocale* l,
    /* [in] */ const String& format,
    /* [in] */ const Array<IInterface*>* args)
{
    return Format(l, format, args);
}

ECode PrintWriter::Format(
    /* [in] */ const String& format,
    /* [in] */ const Array<IInterface*>* args)
{
    ECode ec;
    {
        AutoLock lock(mLock);
        ec = EnsureOpen();
        if (FAILED(ec)) goto ERROR;
        AutoPtr<ILocale> l;
        if ((mFormatter == nullptr) ||
                (mFormatter->GetLocale(l),
                    l != Locale::GetDefault())) {
            mFormatter = nullptr;
            CFormatter::New(this, IID_IFormatter, (IInterface**)&mFormatter);
        }
        ec = mFormatter->Format(Locale::GetDefault(), format, args);
        if (FAILED(ec)) goto ERROR;
        if (mAutoFlush) {
            ec = mOut->Flush();
            if (FAILED(ec)) goto ERROR;
        }
        return NOERROR;
    }

ERROR:
    if (ec == E_INTERRUPTED_IO_EXCEPTION) {
        AutoPtr<IThread> t;
        Thread::GetCurrentThread(&t);
        t->Interrupt();
    }
    else if (ec == E_IO_EXCEPTION) {
        mTrouble = true;
    }
    return NOERROR;
}

ECode PrintWriter::Format(
    /* [in] */ ILocale* l,
    /* [in] */ const String& format,
    /* [in] */ const Array<IInterface*>* args)
{
    ECode ec;
    {
        AutoLock lock(mLock);
        ec = EnsureOpen();
        if (FAILED(ec)) goto ERROR;
        AutoPtr<ILocale> fl;
        if ((mFormatter == nullptr) ||
                (mFormatter->GetLocale(fl), fl != l)) {
            mFormatter = nullptr;
            CFormatter::New(this, l, IID_IFormatter, (IInterface**)&mFormatter);
        }
        ec = mFormatter->Format(l, format, args);
        if (FAILED(ec)) goto ERROR;
        if (mAutoFlush) {
            ec = mOut->Flush();
            if (FAILED(ec)) goto ERROR;
        }
        return NOERROR;
    }

ERROR:
    if (ec == E_INTERRUPTED_IO_EXCEPTION) {
        AutoPtr<IThread> t;
        Thread::GetCurrentThread(&t);
        t->Interrupt();
    }
    else if (ec == E_IO_EXCEPTION) {
        mTrouble = true;
    }
    return NOERROR;
}

}
}
