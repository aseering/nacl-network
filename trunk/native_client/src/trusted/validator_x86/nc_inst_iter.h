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
 * Defines an instruction (decoder) iterator that processes code segments.
 *
 * The typical use is of the form:
 *
 *    NcSegment segment;
 *    NcInstIter* iter;
 *    ...
 *    for (iter = NcInstIterCreate(&segment); NcInstIterHasNext(iter);
 *         NcInstIterAdvance(iter)) {
 *      NcInstState* state = NcInstIterGetState(iter);
 *      ...
 *    }
 *    NcInstIterDestroy(iter);
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NC_INST_ITER_h_
#define NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NC_INST_ITER_h_

#include "native_client/src/trusted/validator_x86/ncopcode_desc.h"
#include "native_client/src/shared/utils/types.h"

/* Defines a code segment in the elf file. */
struct NcSegment;

/* Defines the internal state associated with a parsed instruction.*/
struct NcInstState;

/* Defines the structure of an instruction iterator, which walks
 * the code segment, one instruction at a time.
 */
typedef struct NcInstIter NcInstIter;

/* Defines the corresponding expression tree of the found instruction.
 * See native_client/src/trusted/validator_x86/ncop_exps.h for more
 * details.
 */
struct OpExpressionNode;

/* Allocate and initialize an instruction iterator for the given code segment.
 */
NcInstIter* NcInstIterCreate(struct NcSegment* segment);

/* Allocate and initialize an instruction iterator for the given code segment.
 * Add internal buffering that will allow one to look back the given number
 * of instructions from the current point of the iterator.
 */
NcInstIter* NcInstIterCreateWithLookback(
    struct NcSegment* segment,
    size_t lookback_size);

/* Delete the instruction iterator created by either
 * NcInstIterCreate or NcInstIterCreateWithLookback.
 */
void NcInstIterDestroy(NcInstIter* iter);

/* Return true if there are more instructions in the code segment
 * to iterate over.
 */
Bool NcInstIterHasNext(NcInstIter* iter);

/* Return a state containing the currently matched instruction defined
 * by the given instruction iterator. Note: This instruction is only
 * valid until the next call to NcInstIteratorAdvance.
 */
struct NcInstState* NcInstIterGetState(NcInstIter* iter);

/* Returns true iff it is valid to look back the given distance. */
Bool NcInstIterHasLookbackState(NcInstIter* iter, size_t distance);

/* Return a state containing the instruction matched distance elements ago
 * from the currently matched instruction (zero corresponds to the currently
 * matched instruction). Note: This instruction is only valid until the next
 * call to NcInstIteratorAdvance.
 */
struct NcInstState* NcInstIterGetLookbackState(
    NcInstIter* iter, size_t distance);

/* Advance the iterator past the current instruction. */
void NcInstIterAdvance(NcInstIter* iter);

#endif  /* NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NC_INST_ITER_h_ */
