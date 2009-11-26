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
 * NaCl semaphore cross-platform abstraction
 */
#ifndef NATIVE_CLIENT_SRC_TRUSTED_PLATFORM_NACL_SEMAPHORE_H_
#define NATIVE_CLIENT_SRC_TRUSTED_PLATFORM_NACL_SEMAPHORE_H_


#if NACL_WINDOWS
#include "native_client/src/shared/platform/win/nacl_semaphore.h"
#elif NACL_LINUX
#include "native_client/src/shared/platform/linux/nacl_semaphore.h"
#elif NACL_OSX
#include "native_client/src/shared/platform/osx/nacl_semaphore.h"
#else
#error "Unknown platform!!!"
#endif

int NaClSemCtor(struct NaClSemaphore *sem, int32_t value);

void NaClSemDtor(struct NaClSemaphore *sem);

NaClSyncStatus NaClSemWait(struct NaClSemaphore *sem);

NaClSyncStatus NaClSemTryWait(struct NaClSemaphore *sem);

NaClSyncStatus NaClSemPost(struct NaClSemaphore *sem);

int NaClSemGetValue(struct NaClSemaphore *sem);

#endif  /* NATIVE_CLIENT_SRC_TRUSTED_PLATFORM_NACL_SEMAPHORE_H_ */
