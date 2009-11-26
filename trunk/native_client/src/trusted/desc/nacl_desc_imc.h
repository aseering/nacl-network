/*
 * Copyright 2008, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * NaCl service runtime.  NaClDescImcDesc subclass of NaClDesc.
 */
#ifndef NATIVE_CLIENT_SRC_TRUSTED_DESC_NACL_DESC_IMC_H_
#define NATIVE_CLIENT_SRC_TRUSTED_DESC_NACL_DESC_IMC_H_

#include "native_client/src/include/portability.h"

#include "native_client/src/include/nacl_base.h"

/*
 * get NaClHandle, which is a typedef and not a struct pointer, so
 * impossible to just forward declare.
 */
#include "native_client/src/shared/imc/nacl_imc_c.h"

EXTERN_C_BEGIN

struct NaClDesc;
struct NaClDescEffector;
struct NaClDescImcConnectedDesc;
struct NaClDescImcDesc;
struct NaClDescXferableDataDesc;
struct NaClDescXferState;
struct NaClHostDesc;
struct NaClMessageHeader;
struct nacl_abi_timespec;
struct nacl_abi_stat;

int NaClDescImcConnectedDescCtor(struct NaClDescImcConnectedDesc  *self,
                                 NaClHandle                       h);

int NaClDescImcDescCtor(struct NaClDescImcDesc  *self,
                        NaClHandle              d);

int NaClDescXferableDataDescCtor(struct NaClDescXferableDataDesc  *self,
                                 NaClHandle                       h);

void NaClDescImcConnectedDescDtor(struct NaClDesc *vself);

int NaClDescImcConnectedDescClose(struct NaClDesc          *vself,
                                  struct NaClDescEffector  *effp);

int NaClDescImcConnectedDescExternalizeSize(struct NaClDesc  *vself,
                                            size_t           *nbytes,
                                            size_t           *nhandles);

int NaClDescImcConnectedDescExternalize(struct NaClDesc          *vself,
                                        struct NaClDescXferState *xfer);

int NaClDescImcConnectedDescSendMsg(struct NaClDesc          *vself,
                                    struct NaClDescEffector  *effp,
                                    struct NaClMessageHeader *dgram,
                                    int                      flags);

int NaClDescImcConnectedDescRecvMsg(struct NaClDesc          *vself,
                                    struct NaClDescEffector  *effp,
                                    struct NaClMessageHeader *dgram,
                                    int                      flags);

EXTERN_C_END

#endif  // NATIVE_CLIENT_SRC_TRUSTED_DESC_NACL_DESC_IMC_H_
