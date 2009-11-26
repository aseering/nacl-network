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
 * NaCl testing shell
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#if !NACL_WINDOWS
# include <unistd.h>
# include <sys/types.h>
# include <sys/time.h>
#endif  /* !NACL_WINDOWS */
#ifdef __native_client__
# include <fcntl.h>
# include <nacl/nacl_inttypes.h>
#else
# include "native_client/src/shared/platform/nacl_host_desc.h"
# include "native_client/src/trusted/desc/nacl_desc_base.h"
# include "native_client/src/trusted/service_runtime/include/sys/fcntl.h"
#endif  /* __native_client__ */
#include "native_client/src/shared/srpc/nacl_srpc.h"
#include "native_client/src/shared/srpc/nacl_srpc_internal.h"

/* Table for keeping track of descriptors passed to/from sel_universal */
typedef struct DescList DescList;
struct DescList {
  DescList* next;
  int number;
  const char* print_name;
  NaClSrpcImcDescType desc;
};
static DescList* descriptors = NULL;

static int AddDescToList(NaClSrpcImcDescType new_desc, const char* print_name) {
  static int next_desc = 0;
  DescList* new_list_element;
  DescList** list_pointer;

  /* Return an error if passed a null descriptor */
  if (kNaClSrpcInvalidImcDesc == new_desc) {
    return -1;
  }
  /* Create a descriptor list node. */
  new_list_element = (DescList*) malloc(sizeof(DescList));
  if (NULL == new_list_element) {
    return -1;
  }
  new_list_element->number = next_desc++;
  new_list_element->desc = new_desc;
  new_list_element->print_name = print_name;
  new_list_element->next = NULL;
  /* Find the end of the list of descriptors*/
  for (list_pointer = &descriptors;
       NULL != *list_pointer;
       list_pointer = &(*list_pointer)->next) {
  }
  *list_pointer = new_list_element;
  return new_list_element->number;
}

static NaClSrpcImcDescType DescFromPlatformDesc(int fd, int mode) {
#ifdef __native_client__
  return fd;
#else
  return
      (NaClSrpcImcDescType) NaClDescIoDescMake(NaClHostDescPosixMake(fd, mode));
#endif  /* __native_client__ */
}

static void BuildDefaultDescList() {
#ifdef __native_client__
  const int kRdOnly = O_RDONLY;
  const int kWrOnly = O_WRONLY;
#else
  const int kRdOnly = NACL_ABI_O_RDONLY;
  const int kWrOnly = NACL_ABI_O_WRONLY;
#endif  /* __native_client__ */
  AddDescToList(DescFromPlatformDesc(0, kRdOnly), "stdin");
  AddDescToList(DescFromPlatformDesc(1, kWrOnly), "stdout");
  AddDescToList(DescFromPlatformDesc(2, kWrOnly), "stderr");
}

static void PrintDescList() {
  DescList* list;

  printf("Descriptors:\n");
  for (list = descriptors; NULL != list; list = list->next) {
    printf("  %d: %s\n", list->number, list->print_name);
  }
}

static NaClSrpcImcDescType LookupDesc(int num) {
  DescList* list;

  for (list = descriptors; NULL != list; list = list->next) {
    if (num == list->number) {
      return list->desc;
    }
  }
  return kNaClSrpcInvalidImcDesc;
}

/*  simple destructive tokenizer */
typedef struct {
  const char* start;
  int length;
} TOKEN;

static int HandleEscapedChar(const char** p) {
  int ival;
  int count;

  switch (**p) {
   case '\\':
    ++*p;
    return '\\';
   case '\"':
    ++*p;
    return '\"';
   case 'b':
    ++*p;
    return '\b';
   case 'f':
    ++*p;
    return '\f';
   case 'n':
    ++*p;
    return '\n';
   case 't':
    ++*p;
    return '\t';
   case 'v':
    ++*p;
    return '\v';

   /* Octal sequences. */
   case '0':
   case '1':
   case '2':
   case '3':
   case '4':
   case '5':
   case '6':
   case '7':
    ival = 0;
    count = 0;
    while ('0' <= **p && '7' >= **p && 3 > count) {
      ival = ival * 8 + **p - '0';
      ++*p;
      ++count;
    }
    return ival;

   /* Hexadecimal sequences. */
   case 'x':
   case 'X':
    ival = 0;
    count = 0;
    ++*p;
    while (isxdigit(**p) && 2 > count) {
      if (isdigit(**p)) {
        ival = ival * 16 + **p - '0';
      } else {
        ival = ival * 16 + toupper(**p) - 'A' + 10;
      }
      ++*p;
      ++count;
    }
    return ival;

   default:
    /* Unrecognized token. Return the '\\' and let the caller come back */
    return '\\';
  }
  return -1;
}

/*
 * Reads one char from *p and advances *p to the next read point in the input.
 * This handles some meta characters ('\\', '\"', '\b', '\f', '\n', '\t',
 * and '\v'), \ddd, where ddd is interpreted as an octal character value, and
 * \[xX]dd, where dd is interpreted as a hexadecimal character value.
 */
static int ReadOneChar(const char** p) {
  int ival;

  switch (**p) {
   case '\0':
    /* End of string is returned as -1 so that we can embed '\0' in strings. */
    return -1;
   case '\\':
    ++*p;
    return HandleEscapedChar(p);
   default:
    ival = **p;
    ++*p;
    return ival;
  }
  return -1;
}

/* expects *from to point to leading \" and returns pointer to trailing \" */
static const char* ScanEscapeString(char* to, const char* from) {
  int ival;
  from++;
  ival = ReadOneChar(&from);
  while (-1 != ival) {
    if ('\"' == ival) {
      if (NULL != to) {
        *to = '\0';
      }
      return from;
    }
    if (NULL != to) {
      *to++ = ival;
    }
    ival = ReadOneChar(&from);
  }
  if (NULL != to) {
    *to = ival;
  }
  return 0;
}

static int Tokenize(char* line, TOKEN *array, int n) {
  int pos_start = 0;
  int count = 0;

  for( ; count < n; count++ ) {
    int pos_end;

    /* skip leading white space */
    while (line[pos_start]) {
      const char c = line[pos_start];
      if (isspace(c)) {
        pos_start++;
      } else {
        break;
      }
    }

    if (!line[pos_start]) break;

    /* find token end from current pos_start */
    pos_end = pos_start;

    while (line[pos_end]) {
      const char c = line[pos_end];

      if (isspace(c)) {
        break;
      } else if (c == '\"') {
        /* TBD(sehr): quotes are only really relevant in s("..."). */
        const char* end = ScanEscapeString(0, &line[pos_end]);
        if (!end) return -1;
        pos_end = end - &line[0];
      }
      pos_end++;
    }

    /* save the token */
    array[count].start = &line[pos_start];
    array[count].length = pos_end - pos_start;

    if (line[pos_end]) {
      line[pos_end] = '\0';   /* DESTRUCTION!!! */
      pos_end++;
    }
    pos_start = pos_end;
  }

  return count;
}

/*
 * Create an argument from a token string.  Returns 1 if successful or
 * 0 if not (out of memory or bad type).
 */
static int ParseArg(NaClSrpcArg* arg, const char* token) {
  long val;
  int dim;
  int i;
  const char* comma;

  /* Initialize the argument slot.  This enables freeing on failures. */
  memset(arg, 0, sizeof(*arg));

  dprintf(("TOKEN %s\n", token));
  switch (token[0]) {
   case NACL_SRPC_ARG_TYPE_INVALID:
    arg->tag = NACL_SRPC_ARG_TYPE_INVALID;
    break;
   case NACL_SRPC_ARG_TYPE_BOOL:
    val = strtol(&token[2], 0, 0);
    arg->tag = NACL_SRPC_ARG_TYPE_BOOL;
    arg->u.bval = val;
    break;
   case NACL_SRPC_ARG_TYPE_CHAR_ARRAY:
    dim = strtol(&token[2], 0, 0);
    arg->tag = NACL_SRPC_ARG_TYPE_CHAR_ARRAY;
    arg->u.caval.carr = (char*) calloc(dim, sizeof(char));
    if (NULL == arg->u.caval.carr) {
      return 0;
    }
    arg->u.caval.count = dim;
    comma = strstr(token, ",");
    if (comma) {
      const char* p = comma + 1;
      for (i = 0; *p != ')' && i < dim; ++i) {
        int ival = ReadOneChar(&p);
        if (-1 == ival || ')' == ival) {
          break;
        }
        arg->u.caval.carr[i] = ival;
      }
    }
    break;
   case NACL_SRPC_ARG_TYPE_DOUBLE:
    arg->tag = NACL_SRPC_ARG_TYPE_DOUBLE;
    arg->u.dval = strtod(&token[2], 0);
    break;
   case NACL_SRPC_ARG_TYPE_DOUBLE_ARRAY:
    dim = strtol(&token[2], 0, 0);
    arg->tag = NACL_SRPC_ARG_TYPE_DOUBLE_ARRAY;
    arg->u.daval.darr = (double*) calloc(dim, sizeof(double));
    if (NULL == arg->u.daval.darr) {
      return 0;
    }
    arg->u.daval.count = dim;
    comma = token;
    for (i = 0; i < dim; ++i) {
      comma = strstr(comma, ",");
      if (!comma) break;
      ++comma;
      arg->u.daval.darr[i] = strtod(comma, 0);
    }
    break;
   case NACL_SRPC_ARG_TYPE_HANDLE:
    val = strtol(&token[2], 0, 0);
    arg->tag = NACL_SRPC_ARG_TYPE_HANDLE;
    arg->u.hval = LookupDesc(val);
    break;
   case NACL_SRPC_ARG_TYPE_INT:
    val = strtol(&token[2], 0, 0);
    arg->tag = NACL_SRPC_ARG_TYPE_INT;
    arg->u.ival = val;
    break;
   case NACL_SRPC_ARG_TYPE_INT_ARRAY:
    dim = strtol(&token[2], 0, 0);
    arg->tag = NACL_SRPC_ARG_TYPE_INT_ARRAY;
    arg->u.iaval.iarr = (int*) calloc(dim, sizeof(int));
    if (NULL == arg->u.iaval.iarr) {
      return 0;
    }
    arg->u.iaval.count = dim;
    comma = token;
    for (i = 0; i < dim; ++i) {
      comma = strstr(comma, ",");
      if (!comma) break;
      ++comma;
      arg->u.iaval.iarr[i] = strtol(comma, 0, 0);
    }
    break;
   case NACL_SRPC_ARG_TYPE_STRING:
    arg->tag = NACL_SRPC_ARG_TYPE_STRING;
    /*
     * This is a conservative estimate of the length, as it includes the
     * quotes and possibly some escape characters.
     */
    arg->u.sval = malloc(strlen(token));
    if (NULL == arg->u.sval) {
      return 0;
    }
    ScanEscapeString(arg->u.sval, token + 2);
    break;
    /*
     * The two cases below are added to avoid warnings, they are only used
     * in the plugin code
     */
   case NACL_SRPC_ARG_TYPE_OBJECT:
   case NACL_SRPC_ARG_TYPE_VARIANT_ARRAY:
   default:
    return 0;
  }

  return 1;
}

/*
 * Read n arguments from the tokens array.  Returns n if successful or
 * -1 if not.
 */
static int ParseArgs(NaClSrpcArg* arg, const TOKEN* token, int n) {
  int i;

  for (i = 0; i < n; ++i) {
    if (!ParseArg(&arg[i], token[i].start)) {
      /* TODO(sehr): reclaim memory here on failure. */
      return -1;
    }
  }
  return n;
}

static void PrintOneChar(char c) {
  switch (c) {
   case '\"':
    printf("\\\"");
    break;
   case '\b':
    printf("\\b");
    break;
   case '\f':
    printf("\\f");
    break;
   case '\n':
    printf("\\n");
    break;
   case '\t':
    printf("\\t");
    break;
   case '\v':
    printf("\\v");
    break;
   case '\\':
    printf("\\\\");
    break;
   default:
    if (' ' > c || 127 == c) {
      printf("\\x%02x", c);
    } else {
      printf("%c", c);
    }
  }
}

static void DumpArg(const NaClSrpcArg* arg) {
  uint32_t count;
  uint32_t i;
  char* p;

  switch(arg->tag) {
   case NACL_SRPC_ARG_TYPE_INVALID:
    printf("X()");
    break;
   case NACL_SRPC_ARG_TYPE_BOOL:
    printf("b(%d)", arg->u.bval);
    break;
   case NACL_SRPC_ARG_TYPE_CHAR_ARRAY:
    for (i = 0; i < arg->u.caval.count; ++i)
      PrintOneChar(arg->u.caval.carr[i]);
    break;
   case NACL_SRPC_ARG_TYPE_DOUBLE:
    printf("d(%f)", arg->u.dval);
    break;
   case NACL_SRPC_ARG_TYPE_DOUBLE_ARRAY:
    count = arg->u.daval.count;
    printf("D(%"PRIu32"", count);
    for (i = 0; i < count; ++i)
      printf(",%f", arg->u.daval.darr[i]);
    printf(")");
    break;
   case NACL_SRPC_ARG_TYPE_HANDLE:
    printf("h(%d)", AddDescToList(arg->u.hval, "imported"));
    break;
   case NACL_SRPC_ARG_TYPE_INT:
    printf("i(%d)", arg->u.ival);
    break;
   case NACL_SRPC_ARG_TYPE_INT_ARRAY:
    count = arg->u.iaval.count;
    printf("I(%"PRIu32"", count);
    for (i = 0; i < count; ++i)
      printf(",%d", arg->u.iaval.iarr[i]);
    printf(")");
    break;
   case NACL_SRPC_ARG_TYPE_STRING:
    printf("s(\"");
    for (p = arg->u.sval; '\0' != *p; ++p)
      PrintOneChar(*p);
    printf("\")");
    break;
    /*
     * The two cases below are added to avoid warnings, they are only used
     * in the plugin code
     */
   case NACL_SRPC_ARG_TYPE_OBJECT:
    /* this is a pointer that NaCl module can do nothing with */
    printf("o(%p)", arg->u.oval);
    break;
   case NACL_SRPC_ARG_TYPE_VARIANT_ARRAY:
    count = arg->u.vaval.count;
    printf("A(%"PRIu32"", count);
    for (i = 0; i < count; ++i) {
      printf(",");
      DumpArg(&arg->u.vaval.varr[i]);
    }
    printf(")");
    break;
   default:
    break;
  }
}

static void DumpArgs(const NaClSrpcArg* arg, int n) {
  int i;
  for (i=0; i<n; ++i) {
    printf("  ");
    DumpArg(&arg[i]);
  }
  printf("\n");
}

void BuildArgVec(NaClSrpcArg* argv[], NaClSrpcArg arg[], int count) {
  int i;
  for (i = 0; i < count; ++i) {
    argv[i] = &arg[i];
  }
  argv[count] = NULL;
}

void FreeArrayArgs(NaClSrpcArg arg[], int count) {
  int i;
  for (i = 0; i < count; ++i) {
    switch(arg[i].tag) {
     case NACL_SRPC_ARG_TYPE_CHAR_ARRAY:
      free(arg[i].u.caval.carr);
      break;
     case NACL_SRPC_ARG_TYPE_DOUBLE_ARRAY:
      free(arg[i].u.daval.darr);
      break;
     case NACL_SRPC_ARG_TYPE_INT_ARRAY:
      free(arg[i].u.iaval.iarr);
      break;
     case NACL_SRPC_ARG_TYPE_VARIANT_ARRAY:
      FreeArrayArgs(arg[i].u.vaval.varr, arg[i].u.vaval.count);
      break;
     case NACL_SRPC_ARG_TYPE_INVALID:
     case NACL_SRPC_ARG_TYPE_BOOL:
     case NACL_SRPC_ARG_TYPE_DOUBLE:
     case NACL_SRPC_ARG_TYPE_HANDLE:
     case NACL_SRPC_ARG_TYPE_INT:
     case NACL_SRPC_ARG_TYPE_STRING:
     case NACL_SRPC_ARG_TYPE_OBJECT:
     default:
      break;
    }
  }
}

static void PrintHelp() {
  printf("Commands:\n");
  printf("  # <anything>\n");
  printf("    comment\n");
  printf("  descs\n");
  printf("    print the table of known descriptors (handles)\n");
  printf("  rpc method_name <in_args> * <out_args>\n");
  printf("    -- invoke method_name\n");
  printf("  service\n");
  printf("    print the methods found by service_discovery\n");
  printf("  quit\n");
  printf("    quit the program\n");
  printf("  help\n");
  printf("    print this menu\n");
  printf("  ?\n");
  printf("    print this menu\n");
  /* TODO(sehr,robertm): we should have a syntax description option */
}

static NaClSrpcError UpcallString(NaClSrpcChannel* channel,
                                  NaClSrpcArg** ins,
                                  NaClSrpcArg** outs) {
  printf("UpcallString: called with '%s'\n", ins[0]->u.sval);
  return NACL_SRPC_RESULT_OK;
}

void NaClSrpcCommandLoop(NaClSrpcService* service,
                         NaClSrpcChannel* channel,
                         NaClSrpcInterpreter interpreter,
                         NaClSrpcImcDescType default_socket_address) {
  NaClSrpcError errcode;
  int           command_count = 0;

  /* Add the default descriptors to the table */
  BuildDefaultDescList();
  if (kNaClSrpcInvalidImcDesc != default_socket_address) {
    AddDescToList(default_socket_address, "module socket address");
  }
  /* Add a simple upcall service to the channel (if any) */
  if (NULL != channel) {
    static const NaClSrpcHandlerDesc upcall_handlers[] = {
      { "upcall_string:s:", UpcallString },
      { NULL, NULL }
    };
    NaClSrpcService* service = (NaClSrpcService*) malloc(sizeof(*service));
    if (NULL == service) {
      fprintf(stderr, "Couldn't allocate upcall service\n");
      return;
    }
    if (!NaClSrpcServiceHandlerCtor(service, upcall_handlers)) {
      fprintf(stderr, "Couldn't construct upcall service\n");
      return;
    }
    channel->server = service;
  }
  /* Read RPC requests from stdin and send them. */
  for (;;) {
    char        buffer[4096];
    TOKEN       tokens[NACL_SRPC_MAX_ARGS];
    int         n;
    const char  *command;

    fprintf(stderr, "%d> ", command_count);
    ++command_count;

    if (!fgets(buffer, sizeof(buffer), stdin))
      break;

    n = Tokenize(buffer, tokens, NACL_SRPC_MAX_ARGS);

    if (n < 1) {
      if (n < 0)
        fprintf(stderr, "bad line\n");
      continue;
    }

    command =  tokens[0].start;
    if ('#' == command[0]) {
      continue;
    } else if (0 == strcmp("help", command) ||
               0 == strcmp("?", command)) {
      PrintHelp();
    } else if (0 == strcmp("service", command)) {
      NaClSrpcServicePrint(service);
    } else if (0 == strcmp("descs", command)) {
      PrintDescList();
    } else if (0 == strcmp("quit", command)) {
      break;
    } else if (0 == strcmp("rpc", command)) {
      int          int_out_sep;
      int          n_in;
      NaClSrpcArg  in[NACL_SRPC_MAX_ARGS];
      NaClSrpcArg* inv[NACL_SRPC_MAX_ARGS + 1];
      int          n_out;
      NaClSrpcArg  out[NACL_SRPC_MAX_ARGS];
      NaClSrpcArg* outv[NACL_SRPC_MAX_ARGS + 1];
      uint32_t     rpc_num;

      if (n < 2) {
        fprintf(stderr, "bad rpc command\n");
        continue;
      }

      for (int_out_sep = 2; int_out_sep < n; ++int_out_sep) {
        if (0 == strcmp(tokens[int_out_sep].start, "*"))
          break;
      }

      if (int_out_sep == n) {
        fprintf(stderr, "no in out arg separator for rpc command\n");
        continue;
      }

      /* Build the input parameter values. */
      n_in = int_out_sep - 2;
      dprintf(("parsing in args %d\n", n_in));
      BuildArgVec(inv, in, n_in);

      if (ParseArgs(in, &tokens[2], n_in) < 0) {
        fprintf(stderr, "bad input args for rpc\n");
        continue;
      }

      /* Build the output (rpc return) values. */
      n_out =  n - int_out_sep - 1;
      dprintf(("parsing out args %d\n", n_out));
      BuildArgVec(outv, out, n_out);

      if (ParseArgs(out, &tokens[int_out_sep + 1], n_out) < 0) {
        fprintf(stderr, "bad output args for rpc\n");
        continue;
      }

      rpc_num = NaClSrpcServiceMethodIndex(service, tokens[1].start);
      if (kNaClSrpcInvalidMethodIndex == rpc_num) {
        fprintf(stderr, "unknown rpc\n");
        continue;
      }

      fprintf(stderr, "using rpc %s no %"PRIu32"\n", tokens[1].start, rpc_num);
      errcode = (*interpreter)(service, channel, rpc_num, inv, outv);
      if (NACL_SRPC_RESULT_OK != errcode) {
        fprintf(stderr, "rpc call failed %s\n", NaClSrpcErrorString(errcode));
        continue;
      }

      /* dump result vector */
      printf("%s RESULTS: ", tokens[1].start);
      DumpArgs(outv[0], n_out);

      /* Free the storage allocated for array valued parameters and returns. */
      FreeArrayArgs(in, n_in);
      FreeArrayArgs(out, n_out);
    } else {
        fprintf(stderr, "unknown command\n");
        continue;
    }
  }
}
