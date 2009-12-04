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



#include <sys/nacl_syscalls.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>


int failed(const char *testname, const char *msg) {
  printf("TEST FAILED: %s: %s\n", testname, msg);
  return 0;
}



int passed(const char *testname, const char *msg) {
  printf("TEST PASSED: %s: %s\n", testname, msg);
  return 1;
}





int test_socket() {
  const char* testname = "socket";
  int fd;
  fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(fd < 0)
	  return failed(testname, "socket(PF_INET, SOCK_STREAM, IPPPROTO_TCP)");
  return passed(testname, "all");
}




int testSuite() {
  int ret = 1;
  ret &= test_socket();
/*
 * ret &= test_socketpair();
 * ret &= test_setsockopt();
 *  ret &= test_bind();
 *  ret &= test_connect();
 *  ret &= test_getpeername();
 *  ret &= test_getsockname();
 *  ret &= test_listen();
 *  ret &= test_accept();
 *  ret &= test_recv();
 *  ret &= test_recvfrom();
 *  ret &= test_recvmsg();
 *  ret &= test_send();
 *  //  ret &= test_sendmsg();
 *  //  ret &= test_sendto();
 *  //  ret &= test_shutdown();
 */
  return ret;
}



int main(const int argc, const char *argv[]) {
  int passed;

  passed = testSuite();

  if (passed) {
    printf("All tests PASSED\n");
    printf("yessss\n");
    exit(0);
  } else {
    printf("One or more tests FAILED\n");
    exit(-1);
  }
}
