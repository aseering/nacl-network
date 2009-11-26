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
 * Reads in text file of hexidecimal values, and build a corresponding segment.
 *
 * Note: To see what the segment contains, run ncdis on the corresponding
 * segment to disassemble it.
 *
 * Note: The purpose of this code is to make it easy to specify the contents
 * of code segments using textual values, so that tests are easier to write.
 * The code is NOT industrial strength and shouldn't be used except for simple
 * test cases.
 */

#include <stdio.h>
#include <stdlib.h>

#include "native_client/src/trusted/validator_x86/nc_read_segment.h"

/* Defines the maximum number of characters allowed on an input line
 * of the input text defined by the commands command line option.
 */
#define MAX_INPUT_LINE 4096

static void ConvertHexToByte(char mini_buf[3], int mini_buf_index,
                             uint8_t* mbase, size_t mbase_size,
                             size_t* count) {
  mini_buf[mini_buf_index] = '\0';
  mbase[(*count)++] = (uint8_t)strtoul(mini_buf, NULL, 16);
  if (*count == mbase_size) {
    fprintf(stderr,
            "Error: Hex text file specifies more than %"PRIuS
            " bytes of data\n", mbase_size);
  }
}

/* Given that the first line of Hex text has been read from the given file,
 * and a byte array of the given size, this function read the text of hexidecial
 * values, puts them in the byte array, and returns how many bytes are read.
 * If any line begins with a pound sign (#), it is assumed to be a comment
 * and ignored. If the file contains more hex values that the size of the byte
 * array, an error message is printed and the read is truncated to the size of
 * the byte array. If the number of non-blank hex values aren't even, the single
 * hex value is used as the corresponding byte value.
 */
static size_t NcReadHexData(FILE* file, uint8_t* mbase, size_t mbase_size,
                            char input_line[MAX_INPUT_LINE]) {
  size_t count = 0;
  char mini_buf[3];
  size_t mini_buf_index = 0;
  char *next;
  char ch;
  while (count < mbase_size) {
    if (input_line[0] != '#') {
      /* Not a comment line, process. */
      next = &input_line[0];
      mini_buf_index = 0;
      while (count < mbase_size && (ch = *(next++))) {
        switch (ch) {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
          case 'a':
          case 'b':
          case 'c':
          case 'd':
          case 'e':
          case 'f':
          case 'A':
          case 'B':
          case 'C':
          case 'D':
          case 'E':
          case 'F':
            mini_buf[mini_buf_index++] = ch;
            if (2 == mini_buf_index) {
              ConvertHexToByte(mini_buf, mini_buf_index, mbase,
                               mbase_size, &count);
              if (count == mbase_size) {
                return mbase_size;
              }
              mini_buf_index = 0;
            }
            break;
          default:
            break;
        }
      }
    }
    if (fgets(input_line, MAX_INPUT_LINE, file) == NULL) break;
  }
  if (mini_buf_index > 0) {
    ConvertHexToByte(mini_buf, mini_buf_index, mbase, mbase_size, &count);
  }
  return count;
}

size_t NcReadHexText(FILE* file, uint8_t* mbase, size_t mbase_size) {
  char input_line[MAX_INPUT_LINE];
  if (fgets(input_line, MAX_INPUT_LINE, file) == NULL) return 0;
  return NcReadHexData(file, mbase, mbase_size, input_line);
}

size_t NcReadHexTextWithPc(FILE* file, PcAddress* pc,
                           uint8_t* mbase, size_t mbase_size) {
  char input_line[MAX_INPUT_LINE];
  *pc = 0;  /* Default if no input. */
  while (1) {
    if (fgets(input_line, MAX_INPUT_LINE, file) == NULL) return 0;
    if (input_line[0] == '#') {
      /* i.e. treat line as a comment. */
      continue;
    } else if (input_line[0] == '@') {
      *pc = (PcAddress) strtoul(&input_line[1], NULL, 16);
    } else {
      return NcReadHexData(file, mbase, mbase_size, input_line);
    }
  }
  /* NOT REACHED */
  return 0;
}
