#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */
#include <stdint.h>
#include "nacl_socks_server.h"

#define RCVBUFSIZE 32   /* Size of receive buffer */

uint32_t allowed_ports[] = {7, 80, 1123, 0};

void DieWithError(char *errorMessage);  /* Error handling function */

void HandleTCPClient(int clntSocket)
{
    char echoBuffer[RCVBUFSIZE];        /* Buffer for echo string */
    char sendBuffer[8200];
    uint32_t recvMsgSize;                    /* Size of received message */
    uint32_t nonce;
    uint32_t r; 
    unsigned char hash[20];

    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");

    ParseNaClHashReq(&echoBuffer[0], &nonce, hash);
    // Don't bother checking the hash, we don't care who connects.
    // Just allow anyone.
    printf("Received connection!  Nonce: 0x%x\n", nonce);
    MakeNaClHashResp(&sendBuffer[0], &allowed_ports[0], nonce, 0);
    /* Send received string and receive again until end of transmission */
    /* Echo message back to client */
    printf("Sending...\n");
    if ((r = send(clntSocket, sendBuffer, sizeof(sendBuffer), 0)) != sizeof(sendBuffer)) {
      printf("Size:  %d %d\n", r, recvMsgSize);
      DieWithError("send() failed");
    }
    printf("Sent reply: %d\n", r);

    close(clntSocket);    /* Close client socket */
}
