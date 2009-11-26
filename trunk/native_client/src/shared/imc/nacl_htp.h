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


// NaCl Handle Transfer Protocol
//
// The NaCl sel_ldr uses the NaCl Handle Transfer Protocol to send/receive
// messages between NaCl modules. This module provides the utility functions
// to communicate with a NaCl module from the trusted code.

#ifndef NATIVE_CLIENT_SRC_SHARED_IMC_NACL_HTP_H_
#define NATIVE_CLIENT_SRC_SHARED_IMC_NACL_HTP_H_


#include <stdlib.h>
#ifdef __native_client__
#include <unistd.h>
#include <nacl/nacl_imc.h>
#else
#include "native_client/src/shared/imc/nacl_imc.h"
#endif  // __native_client__

#ifdef __native_client__

namespace nacl {

typedef Handle        HtpHandle;
typedef MessageHeader HtpHeader;

// Creates a host I/O descriptor from the handle.
inline HtpHandle CreateIoDesc(Handle handle) {
  return handle;
}

// Creates a shared memory descriptor from the handle.
inline HtpHandle CreateShmDesc(Handle handle, off_t length) {
  return handle;
}

// Creates an IMC memory descriptor from the handle.
inline HtpHandle CreateImcDesc(Handle handle) {
  return handle;
}

// Reads up to count bytes from the handle into buffer.
inline int Read(HtpHandle handle, void* buffer, size_t count) {
  return read(handle, buffer, count);
}

// Writes up to count bytes to the handle from buffer.
inline int Write(HtpHandle handle, const void* buffer, size_t count) {
  return write(handle, buffer, count);
}

const HtpHandle kInvalidHtpHandle = kInvalidHandle;

}  // namespace nacl

#else  // __native_client__

struct NaClDesc;

namespace nacl {

// NaCl resource descriptor type compatible with sel_ldr.
typedef struct NaClDesc* HtpHandle;

// Message header used by SendDatagram() and ReceiveDatagram().
// Note the member layout is compatible with NaClImcTypedMsgHdr{}.
struct HtpHeader {
  IOVec*     iov;            // scatter/gather array
  size_t     iov_length;     // number of elements in iov
  HtpHandle* handles;        // array of handles to be transferred
  size_t     handle_count;   // number of handles in handles
  int        flags;
};

// Sends a message on a socket. Except for the fact that HtpHeader is used,
// the runtime behavior of this function is the same as SendDatagram() for
// MessageHeader.
// On success, SendDatagram() returns the number of bytes sent.
int SendDatagram(HtpHandle socket, const HtpHeader* message, int flags);

// Receives a message from a socket.  Except for the fact that HtpHeader is
// used, the runtime behavior of this function is the same as ReceiveDatagram()
// for MessageHeader.
// On success, ReceiveDatagram() returns the number of bytes received.
int ReceiveDatagram(HtpHandle socket, HtpHeader* message, int flags);

// Closes a sel_ldr compatible NaCl descriptor.
int Close(HtpHandle handle);

// Maps the specified memory object into the process address space.
// Map() returns a pointer to the mapped area, or kMapFailed upon error.
// For prot, the bitwise OR of the kProt* bits must be specified.
// For flags, either kMapShared or kMapPrivate must be specified. If kMapFixed
// is also set, Map() tries to map the memory object at the address specified by
// start.
void* Map(void* start, size_t length, int prot, int flags,
          HtpHandle memory, off_t offset);

// Creates a host I/O descriptor from the handle.
HtpHandle CreateIoDesc(Handle handle);

// Creates a shared memory descriptor from the handle.
HtpHandle CreateShmDesc(Handle handle, off_t length);

// Creates an IMC memory descriptor from the handle.
HtpHandle CreateImcDesc(Handle handle);

// Reads up to count bytes from the handle into buffer.
int Read(HtpHandle handle, void* buffer, size_t count);

// Writes up to count bytes to the handle from buffer.
int Write(HtpHandle handle, const void* buffer, size_t count);

const HtpHandle kInvalidHtpHandle = NULL;

}  // namespace nacl

#endif  // __native_client__

#endif  // NATIVE_CLIENT_SRC_SHARED_IMC_NACL_HTP_H_
