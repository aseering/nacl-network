#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <sys/nacl_syscalls.h>
#include <fcntl.h>
#include <nacl/nacl_srpc.h>
#include <pthread.h>
#include <errno.h>

#define RCVBUFSIZE 256   /* Size of receive buffer */


static int sock = -1;                        /* Socket descriptor */


static void* ServerThread(void* desc);


NaClSrpcError StartServer(NaClSrpcChannel *channel,
                          NaClSrpcArg **in_args,
                          NaClSrpcArg **out_args) {
  int            pair[2];
  pthread_t      server;
  int            rv;

  /* Create a pair (bound socket, socket address). */
  printf("StartServer: creating bound socket\n");
  rv = imc_makeboundsock(pair);
  if (rv == 0) {
    printf("StartServer: bound socket %d, address %d\n", pair[0], pair[1]);
  } else {
    printf("StartServer: bound socket creation FAILED, errno %d\n", errno);
  }
  /* Pass the socket address back to the caller. */
  out_args[0]->u.hval = pair[1];
  /* Start the server, passing ownership of the bound socket. */
  printf("StartServer: creating server...\n");
  rv = pthread_create(&server, NULL, ServerThread, (void*) pair[0]);
  if (rv == 0) {
    printf("StartServer: server created\n");
  } else {
    printf("StartServer: server creation FAILED, errno %d\n", errno);
  }
  /* Return success. */
  printf("START RPC FINISHED\n");
  return NACL_SRPC_RESULT_OK;
}
NACL_SRPC_METHOD("start_server::h", StartServer);
NaClSrpcError Connect(NaClSrpcChannel *channel,
                         NaClSrpcArg **in_args,
                         NaClSrpcArg **out_args)
{
	struct sockaddr_in echoServAddr; /* Echo server address */
	unsigned short echoServPort;     /* Echo server port */
	char *servIP;                    /* Server IP address (dotted quad) */
	servIP = in_args[0]->u.sval;
	echoServPort = in_args[1]->u.ival;
    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    	goto err;
    }
    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family      = AF_INET;             /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    printf("inet_addr(servIP) = %08x\n", inet_addr(servIP));
    inet_pton(AF_INET, servIP, &echoServAddr.sin_addr.s_addr);
    printf("after inet_pton, s_addr = %08x\n", echoServAddr.sin_addr.s_addr);
    echoServAddr.sin_port        = htons(echoServPort); /* Server port */

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0) {
    	goto err;
    }
    out_args[0]->u.bval = 1;
    return NACL_SRPC_RESULT_OK;
err:
	out_args[0]->u.bval = 0;
	close(sock);
	sock = -1;
	return NACL_SRPC_RESULT_OK;
}
/*NACL_SRPC_METHOD("connect:si:b", Connect);*/

NaClSrpcError Send(NaClSrpcChannel *channel,
                         NaClSrpcArg **in_args,
                         NaClSrpcArg **out_args) {
	char *echoString;                /* String to send to echo server */
	char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
	unsigned int echoStringLen;      /* Length of string to echo */
	int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv() and total bytes read */
	char* ptr;
	echoString = in_args[0]->u.sval;
	printf("echoString = %s\n", echoString);
	echoStringLen = strlen(echoString);          /* Determine input length */
	printf("echoStringLen = %d\n",echoStringLen);
	/* Send the string to the server */
	if (send(sock, echoString, echoStringLen, 0) != echoStringLen) {
		printf("send() sent a different number of bytes than expected");
		goto err;
	}

	/* Receive the same string back from the server */
	totalBytesRcvd = 0;
	ptr = echoBuffer;
	while (totalBytesRcvd < echoStringLen)
	{
		/* Receive up to the buffer size (minus 1 to leave space for
		   a null terminator) bytes from the sender */
		if ((bytesRcvd = recv(sock, ptr, RCVBUFSIZE - 1, 0)) < 0) {
			printf("recv() failed or connection closed prematurely");
			goto err;
		}
		totalBytesRcvd += bytesRcvd;   /* Keep tally of total bytes */
		ptr += bytesRcvd;
		*ptr = '\0';  /* Terminate the string! */
	}

	printf("received: %s\n", echoBuffer);      /* Print the echo buffer */

	/*strncpy(out_args[0]->u.caval.carr, echoBuffer, echoStringLen + 1);*/
	/*strncpy(out_args[0]->u.sval, echoBuffer, strlen(echoBuffer)+1);*/
	out_args[0]->u.sval = strdup(echoBuffer);
	return NACL_SRPC_RESULT_OK;
err:
	out_args[0]->u.sval = strdup("An error occured");
	close(sock);
	sock = -1;
	return NACL_SRPC_RESULT_OK;
}

/*NACL_SRPC_METHOD("send:s:s", Send);*/
NaClSrpcError IsConnected(NaClSrpcChannel *channel,
                         NaClSrpcArg **in_args,
                         NaClSrpcArg **out_args) {
	out_args[0]->u.bval = (sock != -1);
	return NACL_SRPC_RESULT_OK;

}
/*NACL_SRPC_METHOD("isConnected::b", IsConnected);*/

/* Shutdown stops the RPC service. */
static NaClSrpcError Shutdown(NaClSrpcChannel *channel,
                              NaClSrpcArg **in_args,
                              NaClSrpcArg **out_args) {
  printf("ServerThread: Shutdown\n");

  return NACL_SRPC_RESULT_BREAK;
}

static void* ServerThread(void* desc) {
  int connected_desc;

  printf("ServerThread: waiting on %d to accept connections...\n",
         (int) desc);
  /* Wait for connections from the client. */
  connected_desc = imc_accept((int) desc);
  if (connected_desc >= 0) {
    static struct NaClSrpcHandlerDesc methods[] = {
      {"isConnected::b", IsConnected},
      {"sendmsg:s:s", Send},
      { "connecthp:si:b", Connect},
      { "shutdown::", Shutdown } ,
      { NULL, NULL }
    };

    /* Export the server on the connected socket descriptor. */
    if (!NaClSrpcServerLoop(connected_desc, methods, NULL)) {
      printf("SRPC server loop failed.\n");
    }
    printf("ServerThread: shutting down\n");
    /* Close the connected socket */
    if (0 != close(connected_desc)) {
      printf("ServerThread: connected socket close failed.\n");
    }
  } else {
    printf("ServerThread: connection FAILED, errno %d\n", errno);
  }
  /* Close the bound socket */
  if (0 != close((int) desc)) {
    printf("ServerThread: bound socket close failed.\n");
  }
  if (0 != shutdown(sock, SHUT_RDWR)) {
    printf("ServerThread: socket shutdown failed.\n");
  }
  printf("THREAD EXIT\n");
  return 0;
}


