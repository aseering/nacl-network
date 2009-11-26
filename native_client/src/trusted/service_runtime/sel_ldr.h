/*
 * Copyright 2009, Google Inc.
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
 * NaCl Simple/secure ELF loader (NaCl SEL).
 *
 * This loader can only process NaCl object files as produced using
 * the NaCl toolchain.  Other ELF files will be rejected.
 *
 * The primary function, NaClAppLoadFile, parses an ELF file,
 * allocates memory, loads the relocatable image from the ELF file
 * into memory, and performs relocation.  NaClAppRun runs the
 * resultant program.
 *
 * This loader is written in C so that it can be used by C-only as
 * well as C++ applications.  Other languages should also be able to
 * use their foreign-function interfaces to invoke C code.
 *
 * This loader must be part of the NaCl TCB, since it directly handles
 * externally supplied input (the ELF file).  Any security
 * vulnerabilities in handling the ELF image, e.g., buffer or integer
 * overflows, can put the application at risk.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_SERVICE_RUNTIME_SEL_LDR_H__
#define NATIVE_CLIENT_SRC_TRUSTED_SERVICE_RUNTIME_SEL_LDR_H__ 1

#include "native_client/src/include/nacl_base.h"
#include "native_client/src/include/portability.h"
#include "native_client/src/include/elf.h"

#include "native_client/src/shared/platform/nacl_host_desc.h"
#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/shared/platform/nacl_threads.h"

#include "native_client/src/trusted/service_runtime/dyn_array.h"
#include "native_client/src/trusted/service_runtime/nacl_error_code.h"
#include "native_client/src/trusted/service_runtime/nacl_sync_queue.h"
#include "native_client/src/trusted/service_runtime/sel_mem.h"
#include "native_client/src/trusted/service_runtime/sel_util.h"
#include "native_client/src/trusted/service_runtime/sel_rt.h"


EXTERN_C_BEGIN

#define NACL_SERVICE_PORT_DESCRIPTOR    3
#define NACL_SERVICE_ADDRESS_DESCRIPTOR 4

extern int using_debug_configuration;

#define NACL_SELF_CHECK         1

/* wp: NACL_MAX_ADDR_BITS < 32, see NaClAppLoadFile */
#define NACL_DEFAULT_ENTRY_PT   "NaClMain"

#define NACL_DEFAULT_ALLOC_MAX  (32 << 20)  /* total brk and mmap allocs */
#define NACL_DEFAULT_STACK_MAX  (16 << 20)  /* main thread stack */

#define NACL_DATA_SANDBOXING    0
#define NACL_SANDBOX_CHROOT_FD  "SBX_D"
/*
 * If 0, address space is allocated to permit text sandboxing or
 * control flow integrity enforcement via masking.  LDT is used to
 * enforce data access restrictions.
 *
 * If 1, address space is allocated to permit both text and data
 * sandboxing.  Code for this is not as yet written/tested and should
 * be needed only for 64-bit x86 windows.
 */

/*
 * Finds the lowest 1 bit in PF_MASKOS.  Assumes that at least one
 * bit is set, and that this bit is not the highest-order bit.
 *
 * Let us denote PF_MASKOS by n.  Assume n \ne 2^{31}.  Let the k^{th}
 * bit be the lowest order bit that is set, i.e.,
 *   n = m \cdot 2^{k+1} + 2^k, with k,m integers, m \ge 0, and 0 \le k < 31.
 * then (here lhs is C notation, rhs is LaTeX notation):
 *   n ^ (n-1) = (m \cdot 2^{k+1} + 2^k)
 *                 \oplus (m \dot 2^{k+1} + 2^{k-1} + \ldots + 1)
 *             = 2^k + 2^{k-1} + \ldots + 1
 *             = (2^{k+1}-1)
 * so
 *   ((n ^ (n-1)) + 1U) = 2^{k+1}, (since k < 31, no overflow occurs) and
 *   ((n ^ (n-1)) + 1U) >> 1 = 2^k.  QED.
 */
#define PF_OS_WILL_LOAD (((PF_MASKOS ^ (PF_MASKOS-1)) + 1U) >> 1)
#if PF_MASKOS == (1 << 31)
# error "PF_MASKOS too large, invariant needed for PF_OS_WILL_LOAD violated"
#endif

#if NACL_WINDOWS
#define WINDOWS_EXCEPTION_TRY do { __try {
#define WINDOWS_EXCEPTION_CATCH } __except(EXCEPTION_EXECUTE_HANDLER) { \
                                  NaClLog(LOG_ERROR, \
                                      "Unhandled Windows exception\n"); \
                                  exit(1); \
                                } \
                              } while (0)
#else
#define WINDOWS_EXCEPTION_TRY do {
#define WINDOWS_EXCEPTION_CATCH } while (0)
#endif

struct NaClAppThread;

struct NaClApp {
  /*
   * public, user settable.
   */
  uint32_t                  addr_bits;
  uint32_t                  max_data_alloc, stack_size;
  /*
   * max_data_alloc controls how much total data memory can be
   * allocated to the NaCl process; this is initialized data,
   * uninitialized data, and heap and affects the brk system call.
   * the text size and rodata size are not included, even though in
   * NaCl the text and rodata pages are also backed by the pager
   * since due to relocation the text pages and rodata contents
   * cannot simply be memory mapped from the executable.
   *
   * stack_size is the maximum size of the (main) stack.  The stack
   * memory is eager allocated (mapped in w/o MAP_NORESERVE) so
   * there must be enough swap space; page table entries are not
   * populated (no MAP_POPULATE), so actual accesses will likely
   * incur page faults.
   */

  /* determined at load time; OS-determined */
  /* read-only */
  uintptr_t                 mem_start;
  uintptr_t                 xlate_base;

  /* only used for ET_EXEC:  for CS restriction */
  uint32_t                  text_region_bytes;  /* ro. memsz */

  uintptr_t                 data_end;
  /* see break_addr below */

  Elf32_Addr                entry_pt;

  /*
   * Alignment boundary for validation (16 or 32).
   */
  int                       align_boundary;

  /* private */
  Elf32_Ehdr                elf_hdr;

  /*
   * phdrs and sections are mutually exclusive.
   *
   * phdrs non-NULL means that an ELF executable -- with starting text
   * address of NACL_TRAMPOLINE_END -- is used.  sections headers are
   * still loaded, for things like bss size. ???? TODO(bsy)
   *
   * when phdrs is NULL, a relocatable object was used and sections
   * will be non-NULL, with the loader performing relocation as part
   * of the image load.  This is insufficient for C++ since preinit
   * and init code is not executed, so global constructors aren't run,
   * and multiple section groups for template instantiation are not
   * handled properly, among other issues.
   */
  Elf32_Phdr                *phdrs;     /* elf_hdr.e_phnum entries */

  /* common to both ELF executables and relocatable load images */

  uintptr_t                 springboard_addr;  /* relative to mem_start */
  /*
   * springboard code addr for context switching into app sandbox, relative
   * to code sandbox CS
   */

  /*
   * The socket at which the app should be accepting connections.  The
   * corresponding socket address are made available by the JavaScript
   * bridge to other NaCl modules.
   */
  struct NaClDesc           *service_port;
  struct NaClDesc           *service_address;

  struct NaClDesc           *secure_channel;
  struct NaClThread         secure_channel_thr;  /* valid iff secure_channel */

  struct NaClMutex          mu;
  struct NaClCondVar        cv;
  char                      *origin;
  NaClErrorCode             module_load_status;
  int                       module_may_start;

  /*
   * runtime info below, thread state, etc; initialized only when app
   * is run.  Mutex mu protects access to mem_map and other member
   * variables while the application is running and may be
   * multithreaded; thread, desc members have their own locks.  At
   * other times it is assumed that only one thread is
   * constructing/loading the NaClApp and that no mutual exclusion is
   * needed.
   */

  /*
   * memory map is in user addresses.
   */
  struct NaClVmmap          mem_map;

  int                       running;
  int                       exit_status;

  /*
   * enforce that some "special" syscalls may only be made from the
   * main/privileged thread
   */
  int                       restrict_to_main_thread;
  /* all threads enqueue the "special" syscalls to the work queue */
  struct NaClSyncQueue      work_queue;

  uint16_t                  code_seg_sel;
  uint16_t                  data_seg_sel;

  uintptr_t                 break_addr;   /* user addr */
  /* data_end <= break_addr is an invariant */

  int                       freeze_thread_ops;
  /* used when process is killed, or when address space move is needed */

  /*
   * Thread table lock threads_mu is higher in the locking order than
   * the thread locks, i.e., threads_mu must be acqured w/o holding
   * any per-thread lock (natp->mu).
   */
  struct NaClMutex          threads_mu;
  struct NaClCondVar        threads_cv;
  struct DynArray           threads;   /* NaClAppThread pointers */
  int                       num_threads;  /* number actually running */

  struct NaClMutex          desc_mu;
  struct DynArray           desc_tbl;  /* NaClDesc pointers */
};

#define NACL_MAX_PROGRAM_HEADERS  128

enum NaClPhdrCheckAction {
  PCA_NONE,
  PCA_TEXT_CHECK,
  PCA_IGNORE  /* ignore this segment.  currently used only for PT_PHDR. */
};

struct NaClPhdrChecks {
  Elf32_Word                p_type;
  Elf32_Word                p_flags;  /* rwx */
  enum NaClPhdrCheckAction  action;
  int                       required;  /* only for text for now */
  Elf32_Word                p_vaddr;  /* if non-zero, vaddr must be this */
};


void  NaClAppIncrVerbosity(void);

int   NaClAppCtor(struct NaClApp  *nap) NACL_WUR;

void  NaClAppDtor(struct NaClApp  *nap);

void  NaClAppFreeAllMemory(struct NaClApp *nap);

/*
 * Loads a NaCl ELF file into memory in preparation for running it.
 *
 * gp is a pointer to a generic I/O object and should be a GioMem with
 * a memory buffer containing the file read entirely into memory if
 * the file system might be subject to race conditions (e.g., another
 * thread / process might modify a downloaded NaCl ELF file while we
 * are loading it here).
 *
 * nap is a pointer to the NaCl object that is being filled in.  it
 * should be properly constructed via NaClAppCtor.
 *
 * return value: one of the LOAD_* values below.  TODO: add some error
 * detail string and hang that off the nap object, so that more
 * details are available w/o incrementing verbosity (and polluting
 * stdout).
 *
 * note: it may be necessary to flush the icache if the memory
 * allocated for use had already made it into the icache from another
 * NaCl application instance, and the icache does not detect
 * self-modifying code / data writes and automatically invalidate the
 * cache lines.
 */


enum NaClAbiMismatchOption {
  NACL_ABI_MISMATCH_OPTION_ABORT,
  NACL_ABI_MISMATCH_OPTION_IGNORE
};

NaClErrorCode NaClAppLoadFile(struct Gio      *gp,
                              struct NaClApp  *nap,
                              enum NaClAbiMismatchOption abi_mismatch_option)
  NACL_WUR;

size_t  NaClAlignPad(size_t val,
                     size_t align);

void  NaClAppPrintDetails(struct NaClApp  *nap,
                          struct Gio      *gp);

uint32_t  NaClLoad32(uintptr_t    addr);

void  NaClStore32(uintptr_t   addr,
                  uint32_t    v);

NaClErrorCode NaClLoadImage(struct Gio            *gp,
                            struct NaClApp        *nap) NACL_WUR;

void NaClIgnoreValidatorResult();
NaClErrorCode NaClValidateImage(struct NaClApp  *nap) NACL_WUR;


int NaClAddrIsValidEntryPt(struct NaClApp *nap,
                           uintptr_t      addr);

/*
 * Takes ownership of descriptor, i.e., when NaCl app closes, it's gone.
 */
void NaClAddHostDescriptor(struct NaClApp *nap,
                           int            host_os_desc,
                           int            mode,
                           int            nacl_desc);

/*
 * Takes ownership of handle.
 */
void NaClAddImcHandle(struct NaClApp  *nap,
                      NaClHandle      h,
                      int             nacl_desc);

void NaClAddImcAddr(struct NaClApp                  *nap,
                    struct NaClSocketAddress const  *addr,
                    int                             nacl_desc);

/*
 * Used to launch the main thread.  NB: calling thread may in the
 * future become the main NaCl app thread, and this function will
 * return only after the NaCl app main thread exits.  In such an
 * alternative design, NaClWaitForMainThreadToExit will become a
 * no-op.
 */
int NaClCreateMainThread(struct NaClApp     *nap,
                         int                argc,
                         char               **argv,
                         char const *const  *envp) NACL_WUR;

int NaClWaitForMainThreadToExit(struct NaClApp  *nap);

/*
 * Used by syscall code.
 */
int32_t NaClCreateAdditionalThread(struct NaClApp *nap,
                                   uintptr_t      prog_ctr,
                                   uintptr_t      stack_ptr,
                                   uintptr_t      sys_tdb,
                                   size_t         tdb_size) NACL_WUR;

void NaClLoadTrampoline(struct NaClApp *nap);

void NaClLoadTlsHook(struct NaClApp *nap);

void NaClLoadSpringboard(struct NaClApp  *nap);

static const uintptr_t kNaClBadAddress = (uintptr_t) -1;

#ifndef NACL_NO_INLINE
#include "native_client/src/trusted/service_runtime/sel_ldr-inl.h"
#endif

/*
 * Looks up a descriptor in the open-file table.  An additional
 * reference is taken on the returned NaClDesc object (if non-NULL).
 * The caller is responsible for invoking NaClDescUnref() on it when
 * done.
 */
struct NaClDesc *NaClGetDesc(struct NaClApp *nap,
                             int            d);

/*
 * Takes ownership of ndp.
 */
void NaClSetDesc(struct NaClApp   *nap,
                 int              d,
                 struct NaClDesc  *ndp);


int NaClSetAvail(struct NaClApp   *nap,
                 struct NaClDesc  *ndp);

/*
 * Versions that are called while already holding the desc_mu lock
 */
struct NaClDesc *NaClGetDescMu(struct NaClApp *nap,
                               int            d);

void NaClSetDescMu(struct NaClApp   *nap,
                   int              d,
                   struct NaClDesc  *ndp);

int NaClSetAvailMu(struct NaClApp   *nap,
                   struct NaClDesc  *ndp);


int NaClAddThread(struct NaClApp        *nap,
                  struct NaClAppThread  *natp);

int NaClAddThreadMu(struct NaClApp        *nap,
                    struct NaClAppThread  *natp);

void NaClRemoveThread(struct NaClApp  *nap,
                      int             thread_num);

void NaClRemoveThreadMu(struct NaClApp  *nap,
                        int             thread_num);

struct NaClAppThread *NaClGetThreadMu(struct NaClApp  *nap,
                                      int             thread_num);

void NaClAppVmmapUpdate(struct NaClApp    *nap,
                        uintptr_t         page_num,
                        size_t            npages,
                        int               prot,
                        struct NaClMemObj *nmop,
                        int               remove);

uintptr_t NaClAppVmmapFindSpace(struct NaClApp  *nap,
                                int             num_pages);

uintptr_t NaClAppVmmapFindMapSpace(struct NaClApp *nap,
                                   int            num_pages);

void NaClCreateServiceSocket(struct NaClApp *nap);

void NaClSendServiceAddressTo(struct NaClApp  *nap,
                              int             desc);

void NaClSecureCommandChannel(struct NaClApp  *nap);

void NaClDumpServiceAddressTo(struct NaClApp  *nap,
                              int             desc);

void NaClWaitForModuleStartStatusCall(struct NaClApp *nap);

void NaClFillTrampolineRegion(struct NaClApp *nap);

void NaClFillEndOfTextRegion(struct NaClApp *nap);

void NaClPatchOneTrampoline(struct NaClApp *nap,
                            uintptr_t target_addr);
/*
 * target is an absolute address in the source region.  the patch code
 * will figure out the corresponding address in the destination region
 * and modify as appropriate.  this makes it easier to specify, since
 * the target is typically the address of some symbol from the source
 * template.
 */
struct NaClPatch {
  uint32_t            target;
  uint32_t            value;
};

struct NaClPatchInfo {
  uintptr_t           dst;
  uintptr_t           src;
  size_t              nbytes;
  uintptr_t           *rel32;
  size_t              num_rel32;
  struct NaClPatch    *abs32;
  size_t              num_abs32;
  struct NaClPatch    *abs16;
  size_t              num_abs16;
};

void NaClApplyPatchToMemory(struct NaClPatchInfo *patch);

int NaClThreadContextCtor(struct NaClThreadContext  *ntcp,
                          struct NaClApp            *nap,
                          uintptr_t                 prog_ctr,
                          uintptr_t                 stack_ptr,
                          uint32_t                  tls_idx);

void NaClThreadContextDtor(struct NaClThreadContext *ntcp);

#if NACL_ARCH(NACL_BUILD_ARCH) == NACL_x86
static INLINE uintptr_t NaClSandboxAddr(struct NaClApp *nap, uintptr_t addr) {
  return addr & ~(uintptr_t)((1 << nap->align_boundary) - 1);
}
#elif NACL_ARCH(NACL_BUILD_ARCH) == NACL_arm
static INLINE uintptr_t NaClSandboxAddr(struct NaClApp *nap, uintptr_t addr) {
  UNREFERENCED_PARAMETER(nap);

  /*
   * This function is used for sandboxing a user return address before calling
   * into springboard.
   *
   * On ARM, we sandbox the user return address in springboard code, so there is
   * no need to do it here.
   */

  return addr;
}
#else
#error Unknown platform!
#endif

EXTERN_C_END

#endif
