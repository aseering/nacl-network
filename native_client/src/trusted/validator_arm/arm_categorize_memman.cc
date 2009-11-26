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
 * Define a memory manager for handling memory on ARM instruction tries.
 * This API is defined so that we can swap in different memory strategies
 * to deal with the large amount of memory generated by this API.
 */

/* Note: This code has been written to reduce the memory foot print
 * of arm instruction tries. The problem was that the amount of space
 * allocated (in 32-bit compilations) is just about 2.7 Gb's of memory.
 * Hence, we were fighting the wall for memory space. To fix this,
 * we have modified the code to allocate large blocks of memory, and
 * (trivially) allocate space from these large blocks, even at the
 * cost of not being able to free the memory. In exchange, we get
 * back about 0.7Gb's of memory that is lost if we go through the
 * default memory allocator.
 *
 * To minimize this decision, and to be able to change it, we
 * have explicitly defined allocation/free routines for structures
 * ArmInstructionTrie and ArmInstructionList. Therefore, and changes
 * only require rewriting the bodies of these functions.
 *
 * Note: defining macro USE_MALLOC_FOR_TRIE will automatically turn
 * on real memory allocation.
 */

#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <set>

#include "native_client/src/trusted/validator_arm/arm_categorize_memman.h"
#include "native_client/src/trusted/validator_arm/arm_categorize.h"

// When USE_MALLOC_FOR_TRIE is defined, malloc/free's are used to
// control memory allocation. When undefined, we do simple block
// allocations, and don't free up any memory.
#ifndef USE_MALLOC_FOR_TRIE

// Block size to fast allocate memory.
#define ALLOC_BLOCK_SIZE 0x1000

// Holds the pointer to the address of the current allocation block
// from which more memory can be allocated.
static char* trie_block_begin = NULL;

// Holds the pointer to the address of the end of the current allocation
// block.
static char* trie_block_end = NULL;

// Defines the memory size allocation alignment value.
// All addresses and sizes must meet this alignment issue
#define ALLOC_ALIGN sizeof(trie_block_begin)

// Rounds the value up to be divisible by ALLOC_ALIGN
inline size_t AllocAlign(size_t val) {
  size_t overflow = val % ALLOC_ALIGN;
  return overflow ? (val + (val - overflow)) : val;
}

// Keep track of the total amount of memory fast allocated.
static size_t fast_malloc_total = 0;

// Fast allocates a block of memory of the given size. Assumes that
// the argument is a multiple of ALLOC_ALIGN
static void* FastMalloc(size_t size) {
  if (NULL == trie_block_begin ||
      trie_block_begin + size >= trie_block_end) {
    // Allocate more space, no more room in the current block.
    void* new_trie_block_begin =
        mmap(NULL,
             ALLOC_BLOCK_SIZE,
             PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS,
             -1,
             0);
    if (new_trie_block_begin == MAP_FAILED) {
      fprintf(stderr,
              "Fast allocated %u bytes.\n"
              "Unable to allocate more memory, quitting!\n",
              fast_malloc_total);
      exit(1);
    } else {
      fast_malloc_total += ALLOC_BLOCK_SIZE;
      trie_block_begin = reinterpret_cast<char*>(new_trie_block_begin);
    }
    trie_block_end = trie_block_begin + ALLOC_BLOCK_SIZE;

    // Be sure to align block pointer, so that allocations are multiples
    // of alignment.
    size_t offset = reinterpret_cast<size_t>(trie_block_begin) % ALLOC_ALIGN;
    if (offset) {
      trie_block_begin += (static_cast<size_t>(ALLOC_ALIGN) - offset);
    }
  }
  void* ret = trie_block_begin;
  trie_block_begin += size;
  return ret;
}

// Defines allocation size to use for trie nodes.
static const size_t kMallocTrieNodeSize =
  AllocAlign(sizeof(ArmInstructionTrie));

// Defines allocation size to use for instruction list nodes.
static const size_t kMallocInstListNodeSize =
  AllocAlign(sizeof(ArmInstructionList));

// Defines a list of freed instruction trie nodes that can
// be reused for allocation.
static ArmInstructionTrie* free_trie_nodes = NULL;

// Defines a list of freed instruction list nodes that can
// be reused for allocation.
static ArmInstructionList* free_list_nodes = NULL;

#endif

ArmInstructionTrie* MallocArmInstructionTrie() {
#ifdef USE_MALLOC_FOR_TRIE
  return reinterpret_cast<ArmInstructionTrie*>(
      malloc(sizeof(ArmInstructionTrie)));
#else
  if (NULL == free_trie_nodes) {
    return reinterpret_cast<ArmInstructionTrie*>
        (FastMalloc(kMallocTrieNodeSize));
  } else {
    ArmInstructionTrie* new_node = free_trie_nodes;
    free_trie_nodes = free_trie_nodes->parent;
    return new_node;
  }
#endif
}

void FreeArmInstructionTrie(ArmInstructionTrie* node) {
#ifdef USE_MALLOC_FOR_TRIE
  free(node);
#else
  node->parent = free_trie_nodes;
  free_trie_nodes = node;
#endif
}

ArmInstructionList* MallocArmInstructionListNode() {
#ifdef USE_MALLOC_FOR_TRIE
  return reinterpret_cast<ArmInstructionList*>(
      malloc(sizeof(ArmInstructionList)));
#else
  if (NULL == free_list_nodes) {
    return reinterpret_cast<ArmInstructionList*>
        (FastMalloc(kMallocInstListNodeSize));
  } else {
    ArmInstructionList* new_node = free_list_nodes;
    free_list_nodes = free_list_nodes->next;
    return new_node;
  }
#endif
}

void FreeArmInstructionListNode(ArmInstructionList* node) {
#ifdef USE_MALLOC_FOR_TRIE
  free(node);
#else
  node->next = free_list_nodes;
  free_list_nodes = node;
#endif
}

void FreeArmInstructionList(ArmInstructionList* list) {
  while (NULL != list) {
    ArmInstructionList* tmp = list;
    list = list->next;
    FreeArmInstructionListNode(tmp);
  }
}
