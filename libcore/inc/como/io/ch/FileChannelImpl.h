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

#ifndef __COMO_IO_CH_FILECHANNELIMPL_H__
#define __COMO_IO_CH_FILECHANNELIMPL_H__

#include "como.io.IFileDescriptor.h"
#include "como.io.channels.IFileChannel.h"
#include <comosp.h>

using como::io::channels::IFileChannel;

namespace como {
namespace io {
namespace ch {

class FileChannelImpl
{
public:
    static AutoPtr<IFileChannel> Open(
        /* [in] */ IFileDescriptor* fd,
        /* [in] */ const String& path,
        /* [in] */ Boolean readable,
        /* [in] */ Boolean writable,
        /* [in] */ IInterface* parent);

    static AutoPtr<IFileChannel> Open(
        /* [in] */ IFileDescriptor* fd,
        /* [in] */ const String& path,
        /* [in] */ Boolean readable,
        /* [in] */ Boolean writable,
        /* [in] */ Boolean append,
        /* [in] */ IInterface* parent);
};

}
}
}

#endif // __COMO_IO_CH_FILECHANNELIMPL_H__
