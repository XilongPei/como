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

#ifndef __COMO_SECURITY_SECURERANDOM_H__
#define __COMO_SECURITY_SECURERANDOM_H__

#include "como/core/volatile.h"
#include "como/util/Random.h"
#include "como.security.IProvider.h"
#include "como.security.ISecureRandom.h"
#include "como.security.ISecureRandomSpi.h"

using como::util::Random;

namespace como {
namespace security {

class SecureRandom
    : public Random
    , public ISecureRandom
{
public:
    COMO_INTERFACE_DECL();

    ECode Constructor();

    ECode Constructor(
        /* [in] */ const Array<Byte>& seed);

    static ECode GetInstance(
        /* [in] */ const String& algorithm,
        /* [out] */ AutoPtr<ISecureRandom>& sr);

    static ECode GetInstance(
        /* [in] */ const String& algorithm,
        /* [in] */ const String& provider,
        /* [out] */ AutoPtr<ISecureRandom>& sr);

    static ECode GetInstance(
        /* [in] */ const String& algorithm,
        /* [in] */ IProvider* provider,
        /* [out] */ AutoPtr<ISecureRandom>& sr);

    AutoPtr<ISecureRandomSpi> GetSecureRandomSpi();

    ECode GetProvider(
        /* [out] */ AutoPtr<IProvider>& provider) override;

    ECode GetAlgorithm(
        /* [out] */ String& algorithm) override;

    ECode SetSeed(
        /* [in] */ const Array<Byte>& seed) override;

    ECode SetSeed(
        /* [in] */ Long seed) override;

    ECode NextBytes(
        /* [out] */ Array<Byte>& bytes) override;

    static ECode GetSeed(
        /* [in] */ Integer numBytes,
        /* [out, callee] */ Array<Byte>* seed);

    ECode GenerateSeed(
        /* [in] */ Integer numBytes,
        /* [out, callee] */ Array<Byte>* seed) override;

    static SecureRandom* From(
        /* [in] */ ISecureRandom* random);

protected:
    ECode Constructor(
        /* [in] */ ISecureRandomSpi* secureRandomSpi,
        /* [in] */ IProvider* provider);

    Integer Next(
        /* [in] */ Integer bits) override;

private:
    ECode GetDefaultPRNG(
        /* [in] */ Boolean setSeed,
        /* [in] */ const Array<Byte>& seed);

    ECode Constructor(
        /* [in] */ ISecureRandomSpi* secureRandomSpi,
        /* [in] */ IProvider* provider,
        /* [in] */ const String& algorithm);

    static Array<Byte> LongToByteArray(
        /* [in] */ Long l);

    static String GetPrngAlgorithm();

private:
    AutoPtr<IProvider> mProvider;

    AutoPtr<ISecureRandomSpi> mSecureRandomSpi;

    String mAlgorithm;

    VOLATILE static AutoPtr<ISecureRandom> sSeedGenerator;

    friend class CSecureRandom;
};

inline ECode SecureRandom::Constructor(
    /* [in] */ ISecureRandomSpi* secureRandomSpi,
    /* [in] */ IProvider* provider)
{
    return Constructor(secureRandomSpi, provider, String());
}

inline AutoPtr<ISecureRandomSpi> SecureRandom::GetSecureRandomSpi()
{
    return mSecureRandomSpi;
}

inline SecureRandom* SecureRandom::From(
    /* [in] */ ISecureRandom* random)
{
    return (SecureRandom*)random;
}

}
}

#endif // __COMO_SECURITY_SECURERANDOM_H__
