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


/* @file
 *
 * Defines common constants used in ELF files, used by elf32.h, elf64.h
 * and elf.h
 *
 * (Re)Created from the ELF specification at
 * http://x86.ddj.com/ftp/manuals/tools/elf.pdf which is referenced
 * from wikipedia article
 * http://en.wikipedia.org/wki/Executable_and_Linkable_Format
 */

#ifndef NATIVE_CLIENT_SRC_INCLUDE_ELF_CONSTANTS_H_
#define NATIVE_CLIENT_SRC_INCLUDE_ELF_CONSTANTS_H_

#include "native_client/src/include/nacl_base.h"

EXTERN_C_BEGIN

#define EI_NIDENT       16   /* fwd, see rest of EI_* below */

#define ET_NONE         0   /* no file type */
#define ET_REL          1   /* relocatable file */
#define ET_EXEC         2   /* executable file */
#define ET_DYN          3   /* shared object file */
#define ET_CORE         4   /* core file */
/* TODO(karl) figure out effect of adding ET_LOOS through ET_HIOS */
#define ET_LOOS    0xfe00   /* Environment-specific */
#define ET_HIOS    0xfeff   /* Environment-specific */
#define ET_LOPROC  0xff00   /* processor-specific */
#define ET_HIPROC  0xffff   /* processor-specific */

#define EM_NONE         0   /* no machine */
#define EM_M32          1   /* at&t we 32100 */
#define EM_SPARC        2   /* sparc */
#define EM_386          3   /* intel architecture */
#define EM_68K          4   /* motorola 68000 */
#define EM_88K          5   /* motorola 88000 */
#define EM_860          7   /* intel 80860 */
#define EM_MIPS         9   /* mips rs3000 big-endian */
#define EM_MIPS_RS4_BE  10  /* mips rs4000 big-endian */
#define EM_LORESERVED   11
#define EM_HIRESERVED   16
#define EM_ARM          40  /* arm */

#if NACL_ARCH(NACL_BUILD_ARCH) == NACL_x86
#define EM_EXPECTED_BY_NACL EM_386
#elif NACL_ARCH(NACL_BUILD_ARCH) == NACL_arm
#define EM_EXPECTED_BY_NACL EM_ARM
#else
#error Unknown platform!
#endif

#define EV_NONE         0   /* invalid version */
#define EV_CURRENT      1   /* current version */

#define EI_MAG0         0   /* file identification */
#define EI_MAG1         1   /* file identification */
#define EI_MAG2         2   /* file identification */
#define EI_MAG3         3   /* file identification */
#define EI_CLASS        4   /* file class */
#define EI_DATA         5   /* data encoding */
#define EI_VERSION      6   /* file version */
/*
 * EI_PAD deviates from the pdf specification, where its value is 7, since
 * EI_OSABI and EI_ABIVERSION have been introduced.  EI_OSABI and
 * EI_OSABIVERSION are from linux elf.h for code usage compatibility.
 * Also, for Elf 64, the value for EI_PAD is also 9.
 */
#define EI_PAD          9   /* start of padding bytes */

#define EI_OSABI        7
#define EI_ABIVERSION   8

/*
 * ELFMAG and SELFMAG are names/values from linux elf.h, for code usage
 * compatibility.
 */
#define ELFMAG          "\177ELF"
#define SELFMAG         4

/* EI_CLASS values */
#define ELFCLASSNONE    0
#define ELFCLASS32      1
#define ELFCLASS64      2

/* EI_DATA values */
#define ELFDATANONE     0
#define ELFDATA2LSB     1
#define ELFDATA2MSB     2

#define PT_NULL       0           /* Unused entry */
#define PT_LOAD       1           /* Loadable segment */
#define PT_DYNAMIC    2           /* Dynamic linking tables */
#define PT_INTERP     3           /* Program interpreter path name */
#define PT_NOTE       4           /* Note section */
#define PT_SHLIB      5           /* Reserved */
#define PT_PHDR       6           /* Program header table */
#define PT_LOOS       0x60000000  /* Environment-specific low */
#define PT_HIOS       0x6fffffff  /* Environment-specific high */
#define PT_LOPROC     0x70000000  /* Processor-specific low */
#if NACL_ARCH(NACL_BUILD_ARCH) == NACL_arm
#define PT_ARM_EXIDX  0x70000001  /* Exception unwind tables */
#endif
#define PT_HIPROC     0x7fffffff  /* Processor-specific high */
/*
 * PT_TLS and PT_GNU_STACK are from linux elf.h, for code usage
 * compatibility.
 */
#define PT_TLS        7
#define PT_GNU_STACK  0x6474e551

#define PF_X          1
#define PF_W          2
#define PF_R          4
/*
 * PF_MASKOS is from linux elf.h, for code usage compatibility
 */
#define PF_MASKOS     0x0ff00000  /* os specific */

#define SHF_WRITE       0x1         /* Has writable data */
#define SHF_ALLOC       0x2         /* Allocated in memory image of program */
#define SHF_EXECINSTR   0x4         /* Contains executable instructions */
#define SHF_MASKOS      0x0f000000  /* Environment-specific use */
#define SHF_MASKPROC    0xf0000000  /* Processor-specific use */

EXTERN_C_END

#endif  /* NATIVE_CLIENT_SRC_INCLUDE_ELF_CONSTANTS_H_ */