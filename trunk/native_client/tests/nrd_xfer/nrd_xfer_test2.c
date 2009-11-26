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


/** @file
 *
 * Tests the Nrd Xfer protocol implementation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/nacl_imc_api.h>
#include <sys/nacl_syscalls.h>

#include <pthread.h>

void thread_sleep(unsigned int num_seconds) {
  pthread_mutex_t mu;
  pthread_cond_t  cv;
  struct timeval  tv;
  struct timespec ts;

  if (0 == num_seconds) {
    return;
  }
  pthread_mutex_init(&mu, NULL);
  pthread_cond_init(&cv, NULL);

  gettimeofday(&tv, NULL);
  ts.tv_sec = tv.tv_sec + num_seconds;
  ts.tv_nsec = 0;

  pthread_mutex_lock(&mu);
  pthread_cond_timedwait(&cv, &mu, &ts);  /* just for the timeout */
  pthread_mutex_unlock(&mu);

  pthread_cond_destroy(&cv);
  pthread_mutex_destroy(&mu);
}

/*
 * sends a string w/ at most one descriptor.
 */
void send_imc_msg(int         channel,
                  char const  *msg,
                  int         desc) {
  struct NaClImcMsgHdr    msg_hdr;
  struct NaClImcMsgIoVec  iov;
  int                     nbytes;
  int                     rv;

  msg_hdr.iov = &iov;
  msg_hdr.iov_length = 1;
  msg_hdr.descv = &desc;
  msg_hdr.desc_length = -1 != desc;
  iov.base = (char *) msg;
  iov.length = nbytes = strlen(msg);
  if (nbytes != (rv = imc_sendmsg(channel, &msg_hdr, 0))) {
    fprintf(stderr, "imc_sendmsg returned %d, expected %d\n", rv, nbytes);
    fprintf(stderr, "errno %d\n", errno);
    exit(1);
  }
}

int recv_imc_msg(int  channel,
                 char *recvbuf,
                 int  bufsize,
                 int  *descp) {
  struct NaClImcMsgHdr    msg_hdr;
  struct NaClImcMsgIoVec  iov;
  int                     rv;

  msg_hdr.iov = &iov;
  msg_hdr.iov_length = 1;
  msg_hdr.descv = descp;
  msg_hdr.desc_length = NULL != descp;
  msg_hdr.flags = 0;
  iov.base = recvbuf;
  iov.length = bufsize;
  if (-1 == (rv = imc_recvmsg(channel, &msg_hdr, 0))) {
    fprintf(stderr, "imc_recvmsg returned %d\n", rv);
    fprintf(stderr, "errno %d\n", errno);
    exit(1);
  }
  if (0 != (msg_hdr.flags & (RECVMSG_DATA_TRUNCATED
                             | RECVMSG_DESC_TRUNCATED))) {
    fprintf(stderr, "imc_recvmsg truncated: %d\n", msg_hdr.flags);
    exit(1);
  }
  if (msg_hdr.desc_length != (NULL != descp)) {
    fprintf(stderr, "icm_recvmsg: got wrong descriptor count\n");
    exit(1);
  }
  return rv;
}

/*
 * worker thread only processes one fake RPC.
 */
void *worker_thread(void *state) {
  int   d = (int) state;
  char  buf[4096];
  int   rv;
  int   i;
  int   ch;

  ch = imc_accept(d);
  write(2, "accepted\n", 9);

  rv = recv_imc_msg(ch, buf, sizeof buf, NULL);
  /* in case newlib version of stdio has thread safety issues */
  write(2, "Got: ", 5); write(2, buf, rv); write(2, "\n", 1);

  /* echo it ... sdrawkcab! */
  for (i = rv/2; --i >= 0; ) {
    char  t = buf[rv - 1 - i];
    buf[rv - 1 - i] = buf[i];
    buf[i] = t;
  }
  send_imc_msg(ch, buf, -1);
  close(ch);
  close(d);
  return 0;
}

int main(int ac, char **av) {
  int                         opt;
  char const                  *message = NULL;
  char const                  *message2 = NULL;
  int                         server = 0;
  int                         client_desc = -1;
  int                         channel;
  int                         sockaddrd;  /* socket address descriptor */
  int                         subchannel;
  int                         rv;
  char                        data_buffer[4096];
  int                         desc_buffer[IMC_USER_DESC_MAX];
  size_t                      i;
  unsigned long               loop_iter = 1;
  unsigned int                sleep_seconds = 0;

  while (EOF != (opt = getopt(ac, av, "c:l:m:M:sS:t:v"))) {
    switch (opt) {
      case 'c':
        client_desc = strtol(optarg, (char **) 0, 0);
        /* descriptor holds a connection capability for the server */
        break;
      case 'l':
        loop_iter = strtoul(optarg, (char **) 0, 0);
        break;
      case 'm':
        message = optarg;
        break;
      case 'M':
        message2 = optarg;
        break;
      case 's':
        server = 1;
        break;
      case 'S':
        sleep_seconds = strtoul(optarg, (char **) 0, 0);
        break;
      default:
        fprintf(stderr,
                ("Usage: sel_ldr [sel_ldr_args] -- "  /* no newline */
                 "nrd_xfer_test2.nexe [-c server-addr-desc] [-s]\n"
                 "    [-l loop_count] [-S server-sleep-sec]\n"
                 "\n"
                 "Typically, server is run using a command such as\n"
                 "\n"
                 "    sel_ldr -X -1 -D 1 -f nrd_xfer_test2.nexe -- -s\n"
                 "\n"
                 "so the socket address is printed to standard output,\n"
                 "and then the client is run with a command such as\n"
                 "\n"
                 "    sel_ldr -X -1 -a 6:<addr-from-server> " /* no \n */
                 "-- nrd_xfer_test_nacl.nexe -c 6\n"
                 "\n"
                 "to have descriptor 6 be used to represent the socket\n"
                 "address for connecting to the server\n"));
        return 1;
    }
  }

  for (i = 0; i < sizeof desc_buffer / sizeof desc_buffer[0]; ++i) {
    desc_buffer[i] = -1;
  }

  if (server) {
    message = (NULL != message) ? message
        : "\"Hello world!\", from server\n";
    message2 = (NULL != message2) ? message2
        : "SERVER MSG2";
  } else {
    message = (NULL != message) ? message
        : "Client connect request\n";
    message2 = (NULL != message2) ? message2
        :"\"Goodbye cruel world!\", from client\n";
  }

  if (server) {
    int       pair[2];
    pthread_t thr;
    int       err;

    printf("Accepting a client connection...\n");
    channel = imc_accept(3);
    printf("...got channel descriptor %d\n", channel);
    do {

      rv = recv_imc_msg(channel, data_buffer, sizeof data_buffer, NULL);
      printf("Receive returned %d\n", rv);

      if (-1 == rv) {
        fprintf(stderr, "imc_recvmsg failed\n");
        return 1;
      }
      printf("Data bytes: %.*s\n", rv, data_buffer);

      /* send a reply */
      if (-1 == imc_makeboundsock(pair)) {
        fprintf(stderr, "imc_socketpair failed, errno %d\n", errno);
        return 2;
      }
      /*
       * send pair[1], the addr, to peer as reply.
       */
      send_imc_msg(channel, "sockaddr", pair[1]);
      err = pthread_create(&thr, NULL, worker_thread, (void *) pair[0]);
      if (0 != err) {
        fprintf(stderr, "pthread_create failed, returned %d\n", err);
        return 4;
      }

      pthread_detach(thr);

      if (-1 == close(pair[1])) {
        fprintf(stderr, "close of socketpair half failed\n");
      }

      if (0 != sleep_seconds) {
        printf("sleeping for %d seconds...\n", sleep_seconds);
        thread_sleep(sleep_seconds);
      }
    } while (--loop_iter > 0);

    if (-1 == close(channel)) {
      fprintf(stderr, "close of channel %d failed\n", channel);
    }
  } else {
    if (-1 == client_desc) {
      fprintf(stderr,
              "Client needs server socket address to which to connect\n");
      return 100;
    }

    channel = imc_connect(client_desc);

    printf("Connect returned %d\n", channel);

    if (-1 == channel) {
      fprintf(stderr, "Client could not connect, errno %d\n", errno);
      return 101;
    }

    do {
      send_imc_msg(channel, message, -1);
      rv = recv_imc_msg(channel, data_buffer, sizeof data_buffer, &sockaddrd);

      printf("start RPC reply returned socket addr desc %d\n", sockaddrd);
      if (-1 == sockaddrd) {
        fprintf(stderr, "connect failed, errno %d\n", errno);
        return 106;
      }

      subchannel = imc_connect(sockaddrd);

      if (-1 == subchannel) {
        printf("Connect for client-specific socket failed: errno %d\n", errno);
        return 107;
      }

      if (-1 == close(sockaddrd)) {
        fprintf(stderr, "close of %d sockaddr failed, errno %d\n",
                sockaddrd, errno);
      }

      send_imc_msg(subchannel, message2, -1);
      if (-1 == (rv = recv_imc_msg(subchannel, data_buffer, sizeof data_buffer,
                                   NULL))) {
        fprintf(stderr, "receive from worker thread failed, errno %d\n",
                errno);
        return 108;
      }
      /* let's not trust server to NUL terminate */
      data_buffer[sizeof data_buffer - 1] = '\0';
      printf("reply: %s\n", data_buffer);

      if (-1 == close(subchannel)) {
        fprintf(stderr, "close of subchannel %d failed\n", subchannel);
      }
    } while (--loop_iter > 0);

    if (-1 == close(channel)) {
      fprintf(stderr, "close of %d (channel) failed\n", channel);
    }
  }
  return 0;
}
