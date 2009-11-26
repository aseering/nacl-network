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
 * ncfileutil.c - open an executable file. FOR TESTING ONLY.
 */
#include "native_client/src/include/portability.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

#include "native_client/src/include/portability_io.h"

#include "native_client/src/trusted/validator_x86/ncfileutil.h"

/* This module is intended for testing use only, not for production use */
/* in sel_ldr. To prevent unintended production usage, define a symbol  */
/* that will cause a load-time error for sel_ldr.                       */
int gNaClValidateImage_foo = 0;
void NaClValidateImage() { gNaClValidateImage_foo += 1; }

static Elf_Addr NCPageTrunc(Elf_Addr v) {
  return (v & ~(kNCFileUtilPageSize - 1));
}

static Elf_Addr NCPageRound(Elf_Addr v) {
  return(NCPageTrunc(v + (kNCFileUtilPageSize - 1)));
}

/***********************************************************************/
/* THIS ROUTINE IS FOR DEBUGGING/TESTING ONLY, NOT FOR SECURE RUNTIME  */
/* ALL PAGES ARE LEFT WRITEABLE BY THIS LOADER.                        */
/***********************************************************************/
/* Loading a NC executable from a host file */
static off_t readat(const int fd, void *buf, const off_t sz, const size_t at) {
  /* TODO(karl) fix types for off_t and size_t so that the work for 64-bits */
  size_t sofar = 0;
  int nread;
  char *cbuf = (char *)buf;

  if (0 > lseek(fd, at, SEEK_SET)) {
    fprintf(stderr, "readat: lseek failed\n");
    return -1;
  }

  /* TODO(robertm) Figure out if O_BINARY flag fixes this. */
  /* Strangely this loop is needed on Windows. It seems the read()   */
  /* implementation doesn't always return as many bytes as requested */
  /* so you have to keep on trying.                                  */
  do {
    nread = read(fd, &cbuf[sofar], sz - sofar);
    if (nread <= 0) {
      fprintf(stderr, "readat: read failed\n");
      return -1;
    }
    sofar += nread;
  } while (sz != sofar);
  return nread;
}

static int nc_load(ncfile *ncf, int fd, int nc_rules) {

  Elf_Ehdr h;
  ssize_t nread;
  Elf_Addr vmemlo, vmemhi;
  size_t shsize, phsize;
  int i;

  /* Read and check the ELF header */
  nread = readat(fd, &h, sizeof(h), 0);
  if (nread < 0 || (size_t) nread < sizeof(h)) {
    fprintf(stderr, "nc_load(%s): could not read ELF header", ncf->fname);
    return -1;
  }

  /* do a bunch of sanity checks */
  if (strncmp((char *)h.e_ident, ELFMAG, strlen(ELFMAG))) {
    fprintf(stderr, "nc_load(%s): bad magic number", ncf->fname);
    return -1;
  }
  if (h.e_ident[EI_OSABI] != ELFOSABI_NACL) {
    fprintf(stderr, "nc_load(%s): bad OS ABI %x\n",
            ncf->fname, h.e_ident[EI_OSABI]);
    /* return -1; */
  }
  if (h.e_ident[EI_ABIVERSION] != EF_NACL_ABIVERSION) {
    fprintf(stderr, "nc_load(%s): bad ABI version %d\n", ncf->fname,
            h.e_ident[EI_ABIVERSION]);
    /* return -1; */
  }

  if ((h.e_flags & EF_NACL_ALIGN_MASK) == EF_NACL_ALIGN_16) {
    ncf->ncalign = 16;
  } else if ((h.e_flags & EF_NACL_ALIGN_MASK) == EF_NACL_ALIGN_32) {
    ncf->ncalign = 32;
  } else {
    fprintf(stderr, "nc_load(%s): bad align mask %x\n", ncf->fname,
            (uint32_t)(h.e_flags & EF_NACL_ALIGN_MASK));
    ncf->ncalign = 16;
    /* return -1; */
  }

  /* Read the program header table */
  if (h.e_phnum <= 0 || h.e_phnum > kMaxPhnum) {
    fprintf(stderr, "nc_load(%s): h.e_phnum %d > kMaxPhnum %d\n",
            ncf->fname, h.e_phnum, kMaxPhnum);
    return -1;
  }
  ncf->phnum = h.e_phnum;
  ncf->pheaders = (Elf_Phdr *)calloc(h.e_phnum, sizeof(Elf_Phdr));
  if (NULL == ncf->pheaders) {
    fprintf(stderr, "nc_load(%s): calloc(%d, %"PRIdS") failed\n",
            ncf->fname, h.e_phnum, sizeof(Elf_Phdr));
    return -1;
  }
  phsize = h.e_phnum * sizeof(*ncf->pheaders);
  /* TODO(karl) Remove the cast to size_t, or verify size. */
  nread = readat(fd, ncf->pheaders, phsize, (size_t) h.e_phoff);
  if (nread < 0 || (size_t) nread < phsize) return -1;

  /* Iterate through the program headers to find the virtual */
  /* size of loaded text.                                    */
  vmemlo = MAX_ELF_ADDR;
  vmemhi = MIN_ELF_ADDR;
  for (i = 0; i < h.e_phnum; i++) {
    if (ncf->pheaders[i].p_type != PT_LOAD) continue;
    if (0 == (ncf->pheaders[i].p_flags & PF_X)) continue;
    /* This is executable text. Check low and high addrs */
    if (vmemlo > ncf->pheaders[i].p_vaddr) vmemlo = ncf->pheaders[i].p_vaddr;
    if (vmemhi < ncf->pheaders[i].p_vaddr + ncf->pheaders[i].p_memsz) {
      vmemhi = ncf->pheaders[i].p_vaddr + ncf->pheaders[i].p_memsz;
    }
  }
  vmemhi = NCPageRound(vmemhi);
  ncf->size = vmemhi - vmemlo;
  ncf->vbase = vmemlo;
  if (nc_rules && vmemlo != NCPageTrunc(vmemlo)) {
    fprintf(stderr, "nc_load(%s): vmemlo is not aligned\n", ncf->fname);
    return -1;
  }
  /* TODO(karl) Remove the cast to size_t, or verify size. */
  ncf->data = (uint8_t *)calloc(1, (size_t) ncf->size);
  if (NULL == ncf->data) {
    fprintf(stderr, "nc_load(%s): calloc(1, %d) failed\n",
            ncf->fname, (int)ncf->size);
    return -1;
  }

  /* Load program text segments */
  for (i = 0; i < h.e_phnum; i++) {
    const Elf_Phdr *p = &ncf->pheaders[i];
    if (p->p_type != PT_LOAD) continue;
    if (0 == (ncf->pheaders[i].p_flags & PF_X)) continue;

    if (nc_rules) {
      assert(ncf->size >= NCPageRound(p->p_vaddr - ncf->vbase + p->p_memsz));
    }
    /* TODO(karl) Remove the cast to off_t, or verify value in range. */
    nread = readat(fd, &(ncf->data[p->p_vaddr - ncf->vbase]),
                   (off_t) p->p_filesz, (off_t) p->p_offset);
    if (nread < 0 || (size_t) nread < p->p_filesz) {
      fprintf(
          stderr,
          "nc_load(%s): could not read segment %d (%d < %"PRIuElf_Xword")\n",
          ncf->fname, i, (int)nread, p->p_filesz);
      return -1;
    }
  }
  /* load the section headers */
  ncf->shnum = h.e_shnum;
  shsize = ncf->shnum * sizeof(*ncf->sheaders);
  ncf->sheaders = (Elf_Shdr *)calloc(1, shsize);
  if (NULL == ncf->sheaders) {
    fprintf(stderr, "nc_load(%s): calloc(1, %"PRIdS") failed\n",
            ncf->fname, shsize);
    return -1;
  }
  /* TODO(karl) Remove the cast to size_t, or verify value in range. */
  nread = readat(fd, ncf->sheaders, shsize, (size_t) h.e_shoff);
  if (nread < 0 || (size_t) nread < shsize) {
    fprintf(stderr, "nc_load(%s): could not read section headers\n",
            ncf->fname);
    return -1;
  }

  /* success! */
  return 0;
}

ncfile *nc_loadfile_depending(const char *filename, int nc_rules) {
  ncfile *ncf;
  int fd;
  int rdflags = O_RDONLY | _O_BINARY;
  fd = OPEN(filename, rdflags);
  if (fd < 0) return NULL;

  /* Allocate the ncfile structure */
  ncf = calloc(1, sizeof(ncfile));
  if (ncf == NULL) return NULL;
  ncf->size = 0;
  ncf->data = NULL;
  ncf->fname = filename;

  if (nc_load(ncf, fd, nc_rules) < 0) {
    close(fd);
    free(ncf);
    return NULL;
  }
  close(fd);
  return ncf;
}

void nc_freefile(ncfile *ncf) {
  if (ncf->data != NULL) free(ncf->data);
  free(ncf);
}

/***********************************************************************/

void GetVBaseAndLimit(ncfile *ncf, PcAddress *vbase, PcAddress *vlimit) {
  int ii;
  /* TODO(karl) - Define so constant applies to 64-bit pc address. */
  PcAddress base = 0xffffffff;
  PcAddress limit = 0;

  for (ii = 0; ii < ncf->shnum; ii++) {
    if ((ncf->sheaders[ii].sh_flags & SHF_EXECINSTR) == SHF_EXECINSTR) {
      if (ncf->sheaders[ii].sh_addr < base) base = ncf->sheaders[ii].sh_addr;
      if (ncf->sheaders[ii].sh_addr + ncf->sheaders[ii].sh_size > limit)
        limit = ncf->sheaders[ii].sh_addr + ncf->sheaders[ii].sh_size;
    }
  }
  *vbase = base;
  *vlimit = limit;
}
