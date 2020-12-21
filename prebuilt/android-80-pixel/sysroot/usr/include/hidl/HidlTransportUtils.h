/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_HIDL_TRANSPORT_UTILS_H
#define ANDROID_HIDL_TRANSPORT_UTILS_H

#include <android/hidl/base/1.0/IBase.h>

namespace android {
namespace hardware {
namespace details {

/*
 * Verifies the interface chain of 'interface' contains 'castTo'
 * @param emitError if emitError is false, return Return<bool>{false} on error; if emitError
 * is true, the Return<bool> object contains the actual error.
 */
inline Return<bool> canCastInterface(::android::hidl::base::V1_0::IBase* itf,
        const char* castTo, bool emitError = false) {
    if (itf == nullptr) {
        return false;
    }

    bool canCast = false;
    auto chainRet = itf->interfaceChain([&](const hidl_vec<hidl_string> &types) {
        for (size_t i = 0; i < types.size(); i++) {
            if (types[i] == castTo) {
                canCast = true;
                break;
            }
        }
    });

    if (!chainRet.isOk()) {
        // call fails, propagate the error if emitError
        return emitError
                ? details::StatusOf<void, bool>(chainRet)
                : Return<bool>(false);
    }

    return canCast;
}

inline std::string getDescriptor(::android::hidl::base::V1_0::IBase* itf) {
    std::string myDescriptor{};
    auto ret = itf->interfaceDescriptor([&](const hidl_string &types) {
        myDescriptor = types.c_str();
    });
    ret.isOk(); // ignored, return empty string if not isOk()
    return myDescriptor;
}

}   // namespace details
}   // namespace hardware
}   // namespace android

#endif //ANDROID_HIDL_TRANSPORT_UTILS_H
