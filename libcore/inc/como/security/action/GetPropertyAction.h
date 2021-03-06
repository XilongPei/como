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

#ifndef __COMO_SECURITY_ACTION_GETPROPERTYACTION_H__
#define __COMO_SECURITY_ACTION_GETPROPERTYACTION_H__

#include "como/core/SyncObject.h"
#include "como.security.IPrivilegedAction.h"

using como::core::SyncObject;

namespace como {
namespace security {
namespace action {

/**
 * A convenience class for retrieving the string value of a system
 * property as a privileged action.
 *
 * <p>An instance of this class can be used as the argument of
 * <code>AccessController.doPrivileged</code>.
 *
 * <p>The following code retrieves the value of the system
 * property named <code>"prop"</code> as a privileged action: <p>
 *
 * <pre>
 *      AutoPtr<IPrivilegedAction> fsAction;
 *      ECode ec = CGetPropertyAction::New(String("file.separator"),
 *                              IID_IPrivilegedAction, (IInterface**)&fsAction);
 *      AutoPtr<IInterface> fsRet;
 *      ec = AccessController::DoPrivileged(fsAction, fsRet);
 *      mSlash = CoreUtils::Unbox(ICharSequence::Probe(fsRet)).GetChar(0);
 * </pre>
 *
 */
class GetPropertyAction
    : public SyncObject
    , public IPrivilegedAction
{
public:
    COMO_INTERFACE_DECL();

    ECode Constructor(
        /* [in] */ const String& theProp);

    ECode Constructor(
        /* [in] */ const String& theProp,
        /* [in] */ const String& defaultVal);

    ECode Run(
        /* [out] */ AutoPtr<IInterface>& result) override;

private:
    String mTheProp;
    String mDefaultVal;
};

} // namespace action
} // namespace security
} // namespace como

#endif // __COMO_SECURITY_ACTION_GETPROPERTYACTION_H__
