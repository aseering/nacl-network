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


// NaCl inter-module communication primitives.

#ifndef NATIVE_CLIENT_SRC_SHARED_IMC_NACL_IMC_H_
#define NATIVE_CLIENT_SRC_SHARED_IMC_NACL_IMC_H_

/**
 * @file
 * Defines primitive NaCl socket and shared memory functions that provide a
 * portable inter-module communication mechanism between processes on Windows,
 * Mac OS X, and Linux. On Linux, these functions are simple wrapper functions
 * for the AF_UNIX domain socket API.
 *
 * @addtogroup IMC
 * @{
 */

#if NACL_WINDOWS
#include <windows.h>
#endif
#include <sys/types.h>

/**
 * Contains primitive NaCl socket and shared memory functions.
 */
namespace nacl {

/**
 *  @nacl
 *  Gets the last error message string.
 *  @param buffer Pointer to the buffer in which the error message is written.
 *  @param length The size of buffer.
 *  @return 0 on success and -1 on failure.
 */
int GetLastErrorString(char* buffer, size_t length);


/**
 *  @nacl
 *  NaCl resource descriptor type
 */
#if NACL_WINDOWS
typedef HANDLE Handle;
const Handle kInvalidHandle(INVALID_HANDLE_VALUE);
#else
typedef int Handle; /** < PENDING: doc */
const Handle kInvalidHandle(-1); /** < PENDING: doc */
#endif

/**
 *  @nacl
 *  The maximum length of the zero-terminated pathname for SocketAddress
 */
const int kPathMax = 28;            // TBD

/**
 *  @nacl
 *  A NaCl socket address is defined as a pathname. The pathname must be a zero-
 *  terminated character string starting from a character within the ranges A -
 *  Z or a - z. The pathname is not case sensitive on Windows.
 */
struct SocketAddress {
  char path[kPathMax];
};

/**
 *  @nacl
 *  I/O vector for the scatter/gather operation used by SendDatagram() and
 *  ReceiveDatagram()
 */
struct IOVec {
  void*   base;
  size_t  length;
};

/**
 *  @nacl
 *  The maximum number of handles to be passed by SendDatagram()
 */
const size_t kHandleCountMax = 8;   // TBD

/**
 *  @nacl
 *  MessageHeader flags set by ReceiveDatagram()
 */
const int kMessageTruncated = 0x1;  /**< The trailing portion of a message was
                                     *   discarded. */
const int kHandlesTruncated = 0x2;  /**< Not all the handles were received. */

/**
 *  @nacl
 *  Message header used by SendDatagram() and ReceiveDatagram()
 */
struct MessageHeader {
  IOVec*  iov;            /**< scatter/gather array */
  size_t  iov_length;     /**< number of elements in iov */
  Handle* handles;        /**< array of handles to be transferred */
  size_t  handle_count;   /**< number of handles in handles */
  int     flags;
};

/**
 *  @nacl
 *  Creates a NaCl socket associated with the local address.
 *  @param address Pointer to the SocketAddress to bind.
 *  @return A handle of the newly created bound socket on success, and
 *          kInvalidHandle on failure.
 */
Handle BoundSocket(const SocketAddress* address);

/**
 *  @nacl
 *  Creates an unnamed pair of connected sockets.
 *  @param pair Pointer to an array of two Handles in which connected socket
 *              descriptors are returned.
 *  @return 0 on success, and -1 on failure.
 */
int SocketPair(Handle pair[2]);

/**
 *  @nacl
 *  Closes a NaCl descriptor created by the NaCl IMC primitives.
 *  Note NaCl descriptors must be explicitly closed by Close(). Otherwise,
 *  the resources of the underlining operating system will not be released
 *  correctly.
 *  @param handle The NaCl descriptor to close.
 *  @return 0 on success, and -1 on failure.
 */
int Close(Handle handle);

/**
 *  @nacl
 *  SendDatagram()/ReceiveDatagram() flags
 */
const int kDontWait = 0x1;  /**< Enables non-blocking operation */

/**
 *  @nacl
 *  Checks whether the last non-blocking operation failed because no message
 *  was available in the queue.
 *  @return true if the previous non-blocking send or receive operation failure
 *          was because it would block if kDontWait was not specified.
 */
bool WouldBlock();

/**
 *  @nacl
 *  Sends the message to the remote peer of the connection created by
 *  SocketPair().
 *
 *  If kDontWait flag is specified with the call and the other peer of the
 *  socket is unable to receive more data, the function returns -1 without
 *  waiting, and the subsequent WouldBlock() will return true.
 *
 *  Note it is not safe to send messages from the same socket handle by
 *  multiple threads simultaneously.
 *  @param socket The socket descriptor.
 *  @param message Pointer to MessageHeader to send.
 *  @param flags Either 0 or kDontWait.
 *  @return The number of bytes sent, or -1 upon failure
 */
int SendDatagram(Handle socket, const MessageHeader* message, int flags);

/**
 *  @nacl
 *  Sends the message to the socket specified by the name. The socket parameter
 *  should be a socket created by BoundSocket().
 *  @param socket The socket descriptor.
 *  @param message Pointer to MessageHeader to send.
 *  @param flags Either 0 or kDontWait.
 *  @param name The target socket address to which the message is sent.
 *  @return The number of bytes sent, or -1 upon failure
 *  @see SendDatagram()
 */
int SendDatagramTo(Handle socket, const MessageHeader* message, int flags,
                   const SocketAddress* name);

/**
 *  @nacl
 *  Sends the message to the remote peer of the connection created by
 *  SocketPair().
 *
 *  Note it is not safe to send messages from the same socket handle by
 *  multiple threads simultaneously.
 *  @param socket The socket descriptor.
 *  @param buffer Pointer to the data to send.
 *  @param length The length of the data to send.
 *  @param flags Either 0 or kDontWait.
 *  @return The number of bytes sent, or -1 upon failure
 *  @see SendDatagram()
 */
int Send(Handle socket, const void* buffer, size_t length, int flags);

/**
 *  @nacl
 *  Sends the message to the socket specified by the name. The socket parameter
 *  should be a socket created by BoundSocket().
 *  @param socket The socket descriptor.
 *  @param buffer Pointer to the data to send.
 *  @param length The length of the data to send.
 *  @param flags Either 0 or kDontWait.
 *  @param name The target socket address to which the message is sent.
 *  @return The number of bytes sent, or -1 upon failure.
 *  @see SendDatagram()
 */
int SendTo(Handle socket, const void* buffer, size_t length, int flags,
           const SocketAddress* name);

/**
 *  @nacl
 *  Receives a message from a socket.
 *
 *  If kDontWait flag is specified with the call and no messages are available
 *  in the queue, the function returns -1 and the subsequent WouldBlock() will
 *  return true.
 *
 *  Note it is not safe to receive messages from the same socket handle
 *  by multiple threads simultaneously unless the socket handle is created
 *  by BoundSocket().
 *  @param socket The socket descriptor.
 *  @param message Pointer to MessageHeader to receive a message.
 *  @param flags Either 0 or kDontWait.
 *  @return The number of bytes received, or -1 upon failure.
 */
int ReceiveDatagram(Handle socket, MessageHeader* message, int flags);

/**
 *  @nacl
 *  Receives a message from a socket.
 *
 *  Note it is not safe to receive messages from the same socket handle
 *  by multiple threads simultaneously unless the socket handle is created
 *  by BoundSocket().
 *  @param socket The socket descriptor.
 *  @param buffer Pointer to the buffer to receive data.
 *  @param length The length of the buffer to receive data.
 *  @param flags Either 0 or kDontWait.
 *  @return The number of bytes received, or -1 upon failure.
 *  @see ReceiveDatagram()
 */
int Receive(Handle socket, void* buffer, size_t length, int flags);

/**
 *  @nacl
 *  Map allocation granularity
 */
const size_t kMapPageSize = 64 * 1024;

/**
 *  @nacl
 *  Creates a memory object of length bytes.
 *  @param length The size of the memory object to create. It must be a multiple
 *                of allocation granularity given by kMapPageSize.
 *  @return A handle of the newly created memory object on success, and
 *          kInvalidHandle on failure.
 */
Handle CreateMemoryObject(size_t length);

/**
 *  @nacl
 *  Map() prot bits
 */
const int kProtRead = 0x1;    /**< Mapped area can be read */
const int kProtWrite = 0x2;   /**< Mapped area can be written */

/**
 *  @nacl
 *  Map() flags
 */
const int kMapShared = 0x1;   /**< Create a sharable mapping with other
                               *   processes */
const int kMapPrivate = 0x2;  /**< Create a private copy-on-write mapping */
const int kMapFixed = 0x4;    /**< Try to create a mapping at the specified
                               *   address */

void* const kMapFailed = reinterpret_cast<void*>(-1);

/**
 *  @nacl
 *  Maps the specified memory object in the process address space.
 *  @param start The preferred start address of the space range to map if
 *               kMapFixed is set to flags.
 *  @param length The size of the space range to map.
 *  @param prot The bitwise OR of the kProt* bits must be specified.
 *  @param flags Either kMapShared or kMapPrivate must be specified. If
 *               kMapFixed is also set, Map() tries to map the memory object at
 *               the address specified by start.
 *  @param memory The memory object descriptor to map.
 *  @param offset The offset in the memory object from which the memory object
 *                will be mapped.
 *  @return A pointer to the mapped area, and kMapFailed upon error.
 */
void* Map(void* start, size_t length, int prot, int flags,
          Handle memory, off_t offset);

/**
 *  @nacl
 *  Unmaps the memory objects mapped within the specified process address space
 *  range.
 *  @param start The start address of the space range to unmap.
 *  @param length The size of the space range to unmap.
 *  @return 0 on success, and -1 on failure.
 */
int Unmap(void* start, size_t length);

}  // namespace nacl

/**
 * @} End of IMC group
 */

#endif  // NATIVE_CLIENT_SRC_SHARED_IMC_NACL_IMC_H_
