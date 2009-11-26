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
 * NaCl Service Runtime.  I/O Descriptor / Handle abstraction.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_DESC_NACL_DESC_BASE_H_
#define NATIVE_CLIENT_SRC_TRUSTED_DESC_NACL_DESC_BASE_H_


#include "native_client/src/include/nacl_base.h"
#include "native_client/src/include/portability.h"

/* For NaClHandle */
#include "native_client/src/shared/imc/nacl_htp_c.h"

#include "native_client/src/shared/platform/nacl_interruptible_condvar.h"
#include "native_client/src/shared/platform/nacl_interruptible_mutex.h"
#include "native_client/src/shared/platform/nacl_semaphore.h"

EXTERN_C_BEGIN

struct NaClDesc;
struct nacl_abi_stat;
struct nacl_abi_timespec;
struct NaClDescEffector;
struct NaClMessageHeader;

/*
 * Externalization / internalization state, used by
 * Externalize/Internalize functions.  Externalize convert the
 * descriptor represented by the self (this) object to an entry in the
 * handles table in a NaClMessageHeader, and the Internalize function
 * is a factory that takes a dgram and NaClDescXferState and
 * constructs a vector of NaClDesc objects.
 *
 * This is essentially a pair of input/output iterators.  The *_end
 * values are not needed during externalization, since the SendMsg
 * code will have queried ExternalizeSize to ensure that there is
 * enough space.  During internalization, however, we try to be more
 * paranoid and check that we do not overrun our buffers.
 *
 * NB: we must assume that the NaClHandle values passed are the right
 * type; if not, it is possible to violate invariant properties
 * required by the various subclasses of NaClDesc.
 */
struct NaClDescXferState {
  /*
   * In/out value, used for both serialization and deserialization.
   * The Externalize method read/write type tags that are part of the
   * message header as well as data-based capabilities in a
   * self-describing format.
   */
  char        *next_byte;
  char        *byte_buffer_end;

  /*
   * In/out value.  Next handle to work on.
   */
  NaClHandle  *next_handle;
  NaClHandle  *handle_buffer_end;
};

enum NaClDescTypeTag {
  NACL_DESC_DIR,
  NACL_DESC_HOST_IO,
  NACL_DESC_CONN_CAP,
  NACL_DESC_BOUND_SOCKET,
  NACL_DESC_CONNECTED_SOCKET,
  NACL_DESC_SHM,
  NACL_DESC_MUTEX,
  NACL_DESC_CONDVAR,
  NACL_DESC_SEMAPHORE,
  NACL_DESC_TRANSFERABLE_DATA_SOCKET,
  NACL_DESC_IMC_SOCKET
  /*
   * Add new NaCDesc subclasses here.
   *
   * NB: when we add new tag types, NaClDescInternalize[] **MUST**
   * also be updated to add new internalization functions.
   */
};
#define NACL_DESC_TYPE_MAX      (NACL_DESC_IMC_SOCKET + 1)
#define NACL_DESC_TYPE_END_TAG  (0xff)

struct NaClInternalRealHeader {
  uint32_t  xfer_protocol_version;
  uint32_t  descriptor_data_bytes;
};

struct NaClInternalHeader {
  struct NaClInternalRealHeader h;
  /*
   * We add 0x10 here because pad must have at least one element.
   * This unfortunately also means that if NaClInternalRealHeader is
   * already a multiple of 16 in size, we will add in an unnecessary
   * 16-byte pad.  The preprocessor does not have access to sizeof
   * information, so we cannot just get rid of the pad.
   */
  char      pad[((sizeof(struct NaClInternalRealHeader) + 0x10) & ~0xf)
                - sizeof(struct NaClInternalRealHeader)];
  /* total size is a multiple of 16 bytes */
};

#define NACL_HANDLE_TRANSFER_PROTOCOL 0xd3c0de01
/* incr from here */

/*
 * Array of function pointers, indexed by NaClDescTypeTag, any one of
 * which will extract an externalized representation of the NaClDesc
 * subclass object from the message, as referenced via
 * NaClDescXferState, and when successful return the internalized
 * representation -- a newl created NaClDesc subclass object -- to the
 * caller via an out parameter.  Returns 0 on success, negative errno
 * value on failure.
 *
 * NB: we should have atomic failures.  The caller is expected to
 * allocate an array of NaClDesc pointers, and insert into the open
 * file table of the receiving NaClApp (via the NaClDescEffector
 * interface) only when all internalizations succeed.  Since even the
 * insertion can fail, the caller must keep track of the descriptor
 * numbers in case it has to back out and report that the message is
 * dropped.
 *
 * Also, when the NaClDesc object is constructed, the NaClHandle
 * consumed (from the NaClDescXferState) MUST BE replaced with
 * NACL_INVALID_HANDLE.
 */
extern int (*NaClDescInternalize[NACL_DESC_TYPE_MAX])(
    struct NaClDesc **,
    struct NaClDescXferState *);

extern char const *NaClDescTypeString(enum NaClDescTypeTag type_tag);

/*
 * The virtual function table for NaClDesc and its subclasses.
 *
 * This interface will change when non-blocking I/O and epoll is
 * added.
 */

struct NaClDescVtbl {
  void (*Dtor)(struct NaClDesc  *vself);
  uintptr_t (*Map)(struct NaClDesc          *vself,
                   struct NaClDescEffector  *effp,
                   void                     *start_addr,
                   size_t                   len,
                   int                      prot,
                   int                      flags,
                   off_t                    offset) NACL_WUR;
  /*
   * UnmapUnsafe really unmaps and leaves a hole in the address space.
   * It is intended for use by Map (through the effector interface) to
   * clear out memory according to the memory object that is backing
   * the memory, prior to putting new memory in place.  Should not
   * invoke effp->vtbl->UpdateAddrMap.
   */
  int (*UnmapUnsafe)(struct NaClDesc          *vself,
                     struct NaClDescEffector  *effp,
                     void                     *start_addr,
                     size_t                   len) NACL_WUR;
  /*
   * Unmap is the version that removes the mapping but continues to
   * hold the address space in reserve, preventing it from being used
   * accidentally by other threads.  (Address-space squatting.)
   *
   * Should invoke effp->vtbl->UpdateAddrMap.
   */
  int (*Unmap)(struct NaClDesc          *vself,
               struct NaClDescEffector  *effp,
               void                     *start_addr,
               size_t                   len) NACL_WUR;
  ssize_t (*Read)(struct NaClDesc         *vself,
                  struct NaClDescEffector *effp,
                  void                    *buf,
                  size_t                  len) NACL_WUR;
  ssize_t (*Write)(struct NaClDesc          *vself,
                   struct NaClDescEffector  *effp,
                   void const               *buf,
                   size_t                   len) NACL_WUR;
  int (*Seek)(struct NaClDesc         *vself,
              struct NaClDescEffector *effp,
              off_t                   offset,
              int                     whence) NACL_WUR;
  /*
   * TODO(bsy): Need to figure out which requests we support.  Also,
   * request determines arg size and whether it is an input or output arg!
   */
  int (*Ioctl)(struct NaClDesc          *vself,
               struct NaClDescEffector  *effp,
               int                      request,
               void                     *arg) NACL_WUR;
  int (*Fstat)(struct NaClDesc          *vself,
               struct NaClDescEffector  *effp,
               struct nacl_abi_stat     *statbuf);
  int (*Close)(struct NaClDesc          *vself,
               struct NaClDescEffector  *effp) NACL_WUR;

  /*
   * Directory access support.  Directories require support for getdents.
   */
  ssize_t (*Getdents)(struct NaClDesc         *vself,
                      struct NaClDescEffector *effp,
                      void                    *dirp,
                      size_t                  count) NACL_WUR;

  /*
   * typeTag is one of the enumeration values from NaClDescTypeTag.
   *
   * This is not a class variable, since one must access it through an
   * instance.  Having a value in the vtable is not allowed in C++;
   * instead, we would implement this as a virtual function that
   * returns the type tag, or RTTI which would typically be done via
   * examining the vtable pointer.
   */
  enum NaClDescTypeTag typeTag;

  /*
   * Externalization queries this for how many data bytes and how many
   * handles are needed to transfer the "this" or "self" descriptor
   * via IMC.  If the descriptor is not transferrable, this should
   * return -NACL_ABI_EINVAL.  Success is indicated by 0, and other
   * kinds of failure should be the usual negative errno.  Should
   * never have to put the calling thread to sleep or otherwise
   * manipulate thread or process state.
   *
   * The nbytes returned do not include any kind of type tag.  The
   * type tag overhead is computed by the MsgSend code, since tagging
   * format need not be known by the per-descriptor externalization
   * code.
   */
  int (*ExternalizeSize)(struct NaClDesc      *vself,
                         size_t               *nbytes,
                         size_t               *nhandles) NACL_WUR;
  /*
   * Externalize the "this" or "self" descriptor: this will take an
   * IMC datagram object to which the Nrd will be appended, either as
   * special control data or as a descriptor/handle to be passed to
   * the recipient.  Should never have to put the calling thread to
   * sleep or otherwise manipulate thread or process state.
   */
  int (*Externalize)(struct NaClDesc          *vself,
                     struct NaClDescXferState *xfer) NACL_WUR;
  /*
   * Lock and similar syscalls cannot just indefintely block,
   * since address space move will require that all other threads are
   * stopped and in a known
   */
  int (*Lock)(struct NaClDesc         *vself,
              struct NaClDescEffector *effp) NACL_WUR;
  int (*TryLock)(struct NaClDesc          *vself,
                 struct NaClDescEffector  *effp) NACL_WUR;
  int (*Unlock)(struct NaClDesc         *vself,
                struct NaClDescEffector *effp) NACL_WUR;
  int (*Wait)(struct NaClDesc         *vself,
              struct NaClDescEffector *effp,
              struct NaClDesc         *mutex) NACL_WUR;
  int (*TimedWaitAbs)(struct NaClDesc           *vself,
                      struct NaClDescEffector   *effp,
                      struct NaClDesc           *mutex,
                      struct nacl_abi_timespec  *ts) NACL_WUR;
  int (*Signal)(struct NaClDesc         *vself,
                struct NaClDescEffector *effp) NACL_WUR;
  int (*Broadcast)(struct NaClDesc          *vself,
                   struct NaClDescEffector  *effp) NACL_WUR;

  int (*SendMsg)(struct NaClDesc          *vself,
                 struct NaClDescEffector  *effp,
                 struct NaClMessageHeader *dgram,
                 int                      flags) NACL_WUR;
  int (*RecvMsg)(struct NaClDesc          *vself,
                 struct NaClDescEffector  *effp,
                 struct NaClMessageHeader *dgram,
                 int                      flags) NACL_WUR;
  int (*ConnectAddr)(struct NaClDesc          *vself,
                     struct NaClDescEffector  *effp) NACL_WUR;
  int (*AcceptConn)(struct NaClDesc         *vself,
                    struct NaClDescEffector *effp) NACL_WUR;
  int (*Post)(struct NaClDesc         *vself,
              struct NaClDescEffector *effp) NACL_WUR;
  int (*SemWait)(struct NaClDesc          *vself,
                 struct NaClDescEffector  *effp) NACL_WUR;
  int (*GetValue)(struct NaClDesc         *vself,
                  struct NaClDescEffector *effp) NACL_WUR;
  /*
   * Inappropriate methods for the subclass will just return
   * -NACL_ABI_EINVAL.
   */
};

struct NaClDesc {
  struct NaClDescVtbl const *vtbl;
  struct NaClMutex          mu;
  unsigned                  ref_count;
};

/*
 * Placement new style ctor; creates w/ ref_count of 1.
 *
 * The subclasses' ctor must call this base class ctor during their
 * contruction.
 */
int NaClDescCtor(struct NaClDesc *ndp) NACL_WUR;

/* Typically it is incorrect to call the dtor directly; see NaClDescUnref */
void NaClDescDtor(struct NaClDesc *ndp);

struct NaClDesc *NaClDescRef(struct NaClDesc *ndp);

/* when ref_count reaches zero, will call dtor and free */
void NaClDescUnref(struct NaClDesc *ndp);

/*
 * subclasses
 */

/*
 * Directory descriptors
 */
extern struct NaClDescVtbl const kNaClDescDirDescVtbl;

struct NaClDescDirDesc {
  struct NaClDesc           base;
  struct NaClHostDir        *hd;
};

extern int NaClDescDirInternalize(struct NaClDesc          **baseptr,
                                  struct NaClDescXferState *xfer) NACL_WUR;

/*
 * I/O descriptors
 */
extern struct NaClDescVtbl const kNaClDescIoDescVtbl;

struct NaClDescIoDesc {
  struct NaClDesc           base;
  /*
   * No locks are needed for accessing class members; NaClHostDesc
   * should ensure thread safety, and uses are read-only.
   *
   * If we later added state that needs locking, beware lock order.
   */
  struct NaClHostDesc       *hd;
};

extern int NaClDescIoInternalize(struct NaClDesc          **baseptr,
                                 struct NaClDescXferState *xfer) NACL_WUR;

extern struct NaClDescVtbl const kNaClDescConnCapVtbl;

/*
 * IMC socket addresses.
 */
struct NaClDescConnCap {
  struct NaClDesc           base;
  struct NaClSocketAddress  cap;
};

extern int NaClDescConnCapInternalize(struct NaClDesc          **baseptr,
                                      struct NaClDescXferState *xfer) NACL_WUR;

extern struct NaClDescVtbl const kNaClDescImcBoundDescVtbl;

/*
 * IMC bound sockets.
 */
struct NaClDescImcBoundDesc {
  struct NaClDesc           base;
  NaClHandle                h;
};

extern int NaClDescImcBoundDescInternalize(struct NaClDesc          **baseptr,
                                           struct NaClDescXferState *xfer)
NACL_WUR;

extern struct NaClDescVtbl const kNaClDescImcConnectedDescVtbl;
extern struct NaClDescVtbl const kNaClDescImcDescVtbl;
extern struct NaClDescVtbl const kNaClDescXferableDataDescVtbl;

/*
 * IMC connected sockets.  Abstractly, the base class for
 * NaClDescImcDesc and NaClDescXferableDataDesc are identical, with a
 * protected ctor that permits NaClDescImcDescCtor and
 * NaClDescXferableDataDescCtor to set the xferable flag which sets
 * the base class to the appropriate subclass behavior.
 */
struct NaClDescImcConnectedDesc {
  struct NaClDesc           base;
  NaClHandle                h;
};

struct NaClDescImcDesc {
  struct NaClDescImcConnectedDesc base;
  /*
   * race prevention.
   */
  struct NaClMutex          sendmsg_mu;
  struct NaClMutex          recvmsg_mu;
};

struct NaClDescXferableDataDesc {
  struct NaClDescImcConnectedDesc base;
};

extern int NaClDescXferableDataDescInternalize(struct NaClDesc **baseptr,
                                               struct NaClDescXferState *xfer)
NACL_WUR;

extern struct NaClDescVtbl const kNaClDescImcShmVtbl;

struct NaClDescImcShm {
  struct NaClDesc           base;
  NaClHandle                h;
  off_t                     size;  /* note off_t so struct stat compatible */
};

extern int NaClDescImcShmInternalize(struct NaClDesc          **baseptr,
                                     struct NaClDescXferState *xfer) NACL_WUR;

extern struct NaClDescVtbl const kNaClDescMutexVtbl;

struct NaClDescMutex {
  struct NaClDesc      base;
  struct NaClIntrMutex mu;
};

extern int NaClDescMutexInternalize(struct NaClDesc          **baseptr,
                                    struct NaClDescXferState *xfer) NACL_WUR;

extern struct NaClDescVtbl const kNaClDescCondVarVtbl;

struct NaClDescCondVar {
  struct NaClDesc        base;
  struct NaClIntrCondVar cv;
};

extern int NaClDescCondVarInternalize(struct NaClDesc          **baseptr,
                                      struct NaClDescXferState *xfer) NACL_WUR;

extern struct NaClDescVtbl const kNaClDescSemaphoreVtbl;

struct NaClDescSemaphore {
  struct NaClDesc      base;
  struct NaClSemaphore sem;
};

extern int NaClDescSemaphoreInternalize(struct NaClDesc          **baseptr,
                                        struct NaClDescXferState *xfer)
NACL_WUR;

/* utility routines */

struct NaClDescIoDesc *NaClDescIoDescMake(struct NaClHostDesc *nhdp);

/* in PLATFORM/nacl_desc.c */
void NaClDeallocAddrRange(uintptr_t addr,
                          size_t    len);


/* default functions for the vtable - return -NACL_ABI_EINVAL */
void NaClDescDtorNotImplemented(struct NaClDesc  *vself);

uintptr_t NaClDescMapNotImplemented(struct NaClDesc         *vself,
                                    struct NaClDescEffector *effp,
                                    void                    *start_addr,
                                    size_t                  len,
                                    int                     prot,
                                    int                     flags,
                                    off_t                   offset);
int NaClDescUnmapUnsafeNotImplemented(struct NaClDesc         *vself,
                                      struct NaClDescEffector *effp,
                                      void                    *start_addr,
                                      size_t                  len);

int NaClDescUnmapNotImplemented(struct NaClDesc         *vself,
                                struct NaClDescEffector *effp,
                                void                    *start_addr,
                                size_t                  len);
ssize_t NaClDescReadNotImplemented(struct NaClDesc          *vself,
                                   struct NaClDescEffector  *effp,
                                   void                     *buf,
                                   size_t                   len);
ssize_t NaClDescWriteNotImplemented(struct NaClDesc         *vself,
                                    struct NaClDescEffector *effp,
                                    void const              *buf,
                                    size_t                  len);
int NaClDescSeekNotImplemented(struct NaClDesc          *vself,
                               struct NaClDescEffector  *effp,
                               off_t                    offset,
                               int                      whence);
int NaClDescIoctlNotImplemented(struct NaClDesc         *vself,
                                struct NaClDescEffector *natp,
                                int                     request,
                                void                    *arg);
int NaClDescFstatNotImplemented(struct NaClDesc         *vself,
                                struct NaClDescEffector *effp,
                                struct nacl_abi_stat    *statbuf);
int NaClDescCloseNotImplemented(struct NaClDesc         *vself,
                                struct NaClDescEffector *effp);

ssize_t NaClDescGetdentsNotImplemented(struct NaClDesc          *vself,
                                       struct NaClDescEffector  *effp,
                                       void                     *dirp,
                                       size_t                   count);

int NaClDescExternalizeSizeNotImplemented(struct NaClDesc      *vself,
                                          size_t               *nbytes,
                                          size_t               *nhandles);

int NaClDescExternalizeNotImplemented(struct NaClDesc          *vself,
                                      struct NaClDescXferState *xfer);

int NaClDescLockNotImplemented(struct NaClDesc          *vself,
                               struct NaClDescEffector  *effp);
int NaClDescTryLockNotImplemented(struct NaClDesc         *vself,
                                  struct NaClDescEffector *effp);
int NaClDescUnlockNotImplemented(struct NaClDesc          *vself,
                                 struct NaClDescEffector  *effp);
int NaClDescWaitNotImplemented(struct NaClDesc          *vself,
                               struct NaClDescEffector  *effp,
                               struct NaClDesc          *mutex);
int NaClDescTimedWaitAbsNotImplemented(struct NaClDesc          *vself,
                                       struct NaClDescEffector  *effp,
                                       struct NaClDesc          *mutex,
                                       struct nacl_abi_timespec *ts);
int NaClDescSignalNotImplemented(struct NaClDesc          *vself,
                                 struct NaClDescEffector  *effp);
int NaClDescBroadcastNotImplemented(struct NaClDesc         *vself,
                                    struct NaClDescEffector *effp);

int NaClDescSendMsgNotImplemented(struct NaClDesc           *vself,
                                  struct NaClDescEffector   *effp,
                                  struct NaClMessageHeader  *dgram,
                                  int                       flags);
int NaClDescRecvMsgNotImplemented(struct NaClDesc           *vself,
                                  struct NaClDescEffector   *effp,
                                  struct NaClMessageHeader  *dgram,
                                  int                       flags);
int NaClDescConnectAddrNotImplemented(struct NaClDesc         *vself,
                                      struct NaClDescEffector *effp);
int NaClDescAcceptConnNotImplemented(struct NaClDesc          *vself,
                                     struct NaClDescEffector  *effp);
int NaClDescPostNotImplemented(struct NaClDesc          *vself,
                               struct NaClDescEffector  *effp);
int NaClDescSemWaitNotImplemented(struct NaClDesc         *vself,
                                  struct NaClDescEffector *effp);
int NaClDescGetValueNotImplemented(struct NaClDesc          *vself,
                                   struct NaClDescEffector  *effp);


int NaClDescInternalizeNotImplemented(struct NaClDesc **baseptr,
                                      struct NaClDescXferState *xfer);

int NaClDescMapDescriptor(struct NaClDesc *desc,
                          struct NaClDescEffector *effector,
                          void** addr,
                          size_t* size);

EXTERN_C_END

#endif  // NATIVE_CLIENT_SRC_TRUSTED_DESC_NACL_DESC_BASE_H_
