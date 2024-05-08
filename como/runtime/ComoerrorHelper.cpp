//=========================================================================
// Copyright (C) 2024 The C++ Component Model(COMO) Open Source Project
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

#include "comotypes.h"
#include "comoerror.h"
#include "ComoerrorHelper.h"

namespace como {

typedef struct tagG_ComoEcErrors {
    ComoEcError *ecErrors;
    int num;
    struct tagG_ComoEcErrors *next;
} G_ComoEcErrors;

static ComoEcError comoEcErrors[] = {
    {E_FAILED_EXCEPTION, "E_FAILED_EXCEPTION"},                                 // (0, 0x01)
    {E_ILLEGAL_ARGUMENT_EXCEPTION, "E_ILLEGAL_ARGUMENT_EXCEPTION"},             // (0, 0x02)
    {E_COMPONENT_NOT_FOUND_EXCEPTION, "E_COMPONENT_NOT_FOUND_EXCEPTION"},       // (0, 0x03)
    {E_COMPONENT_UNLOAD_EXCEPTION, "E_COMPONENT_UNLOAD_EXCEPTION"},             // (0, 0x04)
    {E_COMPONENT_IO_EXCEPTION, "E_COMPONENT_IO_EXCEPTION"},                     // (0, 0x05)
    {E_UNSUPPORTED_OPERATION_EXCEPTION, "E_UNSUPPORTED_OPERATION_EXCEPTION"},   // (0, 0x06)
    {E_NOT_FOUND_EXCEPTION, "E_NOT_FOUND_EXCEPTION"},                           // (0, 0x07)
    {E_RUNTIME_EXCEPTION, "E_RUNTIME_EXCEPTION"},                               // (0, 0x08)
    {E_REMOTE_EXCEPTION, "E_REMOTE_EXCEPTION"},                                 // (0, 0x09)
    {E_INTERFACE_NOT_FOUND_EXCEPTION, "E_INTERFACE_NOT_FOUND_EXCEPTION"},       // (0, 0x0a)
    {E_CLASS_NOT_FOUND_EXCEPTION, "E_CLASS_NOT_FOUND_EXCEPTION"},               // (0, 0x0b)
    {E_TYPE_MISMATCH_EXCEPTION, "E_TYPE_MISMATCH_EXCEPTION"},                   // (0, 0x0c)
    {E_IOATTRIBUTE_MISMATCH_EXCEPTION, "E_IOATTRIBUTE_MISMATCH_EXCEPTION"},     // (0, 0x0d)
    {E_NOT_COMO_OBJECT_EXCEPTION, "E_NOT_COMO_OBJECT_EXCEPTION"},               // (0, 0x0e)
    {E_INVALID_SIGNATURE_EXCEPTION, "E_INVALID_SIGNATURE_EXCEPTION"},           // (0, 0x0f)
    {E_TOO_MANY_CONNECTION_EXCEPTION, "E_TOO_MANY_CONNECTION_EXCEPTION"},       // (0, 0x10)
    {E_CONST_NOT_FOUND_EXCEPTION, "E_CONST_NOT_FOUND_EXCEPTION"},               // (0, 0x11)

    {E_OUT_OF_MEMORY_ERROR, "E_OUT_OF_MEMORY_ERROR"}                            // (0, 0xf0)
};

G_ComoEcErrors g_ComoEcErrors = {comoEcErrors,
                         sizeof(comoEcErrors)/sizeof(ComoEcError), nullptr};
G_ComoEcErrors *g_pComoEcErrors = &g_ComoEcErrors;

/**
 * GetEcErrorInfo
 */
const char *ComoerrorHelper::GetEcErrorInfo(ECode ec)
{
    G_ComoEcErrors *errs = g_pComoEcErrors;
    while (nullptr != errs) {
        for (int i = 0;  i < errs->num;  i++) {
            if (ec == errs->ecErrors[i].ec) {
                return errs->ecErrors[i].info;
            }
        }

        errs = errs->next;
    }

    return (const char *)"";
}

/**
 * RegisterEcErrors
 *
 * ecErrors should be an array of ComoEcError.
 */
int ComoerrorHelper::RegisterEcErrors(ComoEcError *ecErrors, int num)
{

    G_ComoEcErrors *ers = new G_ComoEcErrors;
    if (nullptr == ers) {
        return -1;
    }

    ers->ecErrors = ecErrors;
    ers->num = num;
    ers->next = g_pComoEcErrors;
    g_pComoEcErrors = ers;

    return 0;
}

} // namespace como
