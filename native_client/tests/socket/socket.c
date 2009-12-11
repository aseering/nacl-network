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
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

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

#define RCVBUFSIZE 10000

int test_open_conn() {

    int sock;                        /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short echoServPort;     /* Echo server port */
    char *servIP;                    /* Server IP address (dotted quad) */
    char *echoString;                /* String to send to echo server */
    char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
    unsigned int echoStringLen;      /* Length of string to echo */
    int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv() 
                                        and total bytes read */

    servIP = "18.208.0.160";
    echoString = "Test Echo";

    echoServPort = 7;  /* 7 is the well-known port for the echo service */

    printf("Just before socket() call\n");

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
      return failed("open_conn", "socket");

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family      = AF_INET;             /* Internet address family */
    inet_pton(AF_INET, servIP, &echoServAddr.sin_addr.s_addr);   /* Server IP address */
    echoServAddr.sin_port        = htons(echoServPort); /* Server port */
    printf("ENDIAN:  %d %d %d %hx %hx\n", __BIG_ENDIAN, __LITTLE_ENDIAN, __BYTE_ORDER, echoServPort, echoServAddr.sin_port);

    printf("connect test:  IP 0x%lx\n", (uint32_t)echoServAddr.sin_addr.s_addr);
    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
      return failed("open_conn", "connect");

    echoStringLen = strlen(echoString);          /* Determine input length */

    /* Send the string to the server */
    if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
      return failed("open_conn", "send");

    /* Receive the same string back from the server */
    totalBytesRcvd = 0;
    printf("Received: ");                /* Setup to print the echoed string */
    while (totalBytesRcvd < echoStringLen)
    {
        /* Receive up to the buffer size (minus 1 to leave space for
           a null terminator) bytes from the sender */
        if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
	  return failed("open_conn", "recv");
        totalBytesRcvd += bytesRcvd;   /* Keep tally of total bytes */
        echoBuffer[bytesRcvd] = '\0';  /* Terminate the string! */
        printf("%s", echoBuffer);      /* Print the echo buffer */
    }

    printf("\n");    /* Print a final linefeed */

    close(sock);

    return passed("open_conn", "all");
}


int testSuite() {
  int ret = 1;
  ret &= test_socket();
  printf("Done with testSuite; testing test_open_conn.\n");
  ret &= test_open_conn();
  printf("Done with test_open_conn()\n");
  
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
