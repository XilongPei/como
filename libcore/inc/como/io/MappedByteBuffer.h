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

#ifndef __COMO_IO_MAPPEDBYTEBUFFER_H__
#define __COMO_IO_MAPPEDBYTEBUFFER_H__

#include "como/io/ByteBuffer.h"
#include "como.io.IFileDescriptor.h"
#include "como.io.IMappedByteBuffer.h"

namespace como {
namespace io {

class MappedByteBuffer
    : public ByteBuffer
    , public IMappedByteBuffer
{
public:
    COMO_INTERFACE_DECL();

    ECode Constructor(
        /* [in] */ Integer mark,
        /* [in] */ Integer pos,
        /* [in] */ Integer lim,
        /* [in] */ Integer cap,
        /* [in] */ IFileDescriptor* fd);

    ECode Constructor(
        /* [in] */ Integer mark,
        /* [in] */ Integer pos,
        /* [in] */ Integer lim,
        /* [in] */ Integer cap,
        /* [in] */ Array<Byte>& buf,
        /* [in] */ Integer offset);

    ECode Constructor(
        /* [in] */ Integer mark,
        /* [in] */ Integer pos,
        /* [in] */ Integer lim,
        /* [in] */ Integer cap);

    ECode IsLoaded(
        /* [out] */ Boolean& loaded) override final;

    ECode Load() override final;

    ECode Force() override final;

private:
    ECode CheckMapped();

    HANDLE MappingOffset();

    HANDLE MappingAddress(
        /* [in] */ HANDLE mappingOffset);

    Long MappingLength(
        /* [in] */ HANDLE mappingOffset);

    ECode IsLoaded0(
        /* [in] */ HANDLE address,
        /* [in] */ Long length,
        /* [in] */ Integer pageCount,
        /* [out] */ Boolean& loaded);

    ECode Load0(
        /* [in] */ HANDLE address,
        /* [in] */ Long length);

    ECode Force0(
        /* [in] */ IFileDescriptor* fd,
        /* [in] */ HANDLE address,
        /* [in] */ Long length);

private:
    AutoPtr<IFileDescriptor> mFd;

    static Byte sUnused;
};

}
}

#endif // __COMO_IO_MAPPEDBYTEBUFFER_H__
