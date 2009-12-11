#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>

#include "native_client/src/trusted/service_runtime/linux/nacl_socks_client.h"
#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"

#define NUM_REMOTE_SERVER_PORTS 20
struct NaClRemoteServerPorts remoteServerPortsRingbuffer[NUM_REMOTE_SERVER_PORTS];
int rsprb_index = 0;


int verifyServerPort(struct NaClRemoteServerPorts *p, uint16_t port) {
  return (((p->ports[port/8]) & (1 << (port % 8))) != 0);
}


int NaClIsConnectionOk(const struct sockaddr *addr, unsigned char* hash) {
  // We've verified that this must (well, is supposed to) be an inet struct
  struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;

  int i, r;

  unsigned char buf[24];
  char ret_buf[8200];
  struct NaClRemoteServerPorts *remote_ports;

  struct sockaddr_in to;
  int sockfd;

  printf("Allocated memory.  Are we AF_INET?\n");
  if (addr->sa_family != AF_INET) {
    return -1; // We don't support anything != IPv4 at this time
  }

  printf("Checking buffer\n");
  // See if we already know what to do with this IP
  for (i = 0; i < NUM_REMOTE_SERVER_PORTS; i++) {
    printf("Buffer #%d\n", i);
    if (remoteServerPortsRingbuffer[i].ip == addr_in->sin_addr.s_addr) {
      return verifyServerPort(&remoteServerPortsRingbuffer[i], addr_in->sin_port);
    }
  }

  // We don't.  So, go fetch a response, and use it.
  // Somehow, we have to get our own hash.
  // For now, just hardcode a magic value.
  printf("Go make a hash-thing\n");
  MakeNaClHashReq(&buf[0], hash, 0);
  

  printf("Setting up &from\n");
  to.sin_port = htons(NACL_VALIDATE_SERVERPORT);
  to.sin_family = AF_INET;
  to.sin_addr = addr_in->sin_addr;
  /*printf("Set up some in-memory data structures\n");
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  
  printf("Getaddrinfo\n");
  sprintf(&itoa_buf[0], "%d", NACL_VALDATE_SERVERPORT);
  if ((r = getaddrinfo(NULL, &itoa_buf[0], &hints, &servinfo)) != 0) {
    return -1;
    }*/
  
  
  printf("socket\n");
  if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)  {
    printf("socket value: %d\n", sockfd);
    return sockfd;
  }

  if ((r = connect(sockfd, &to, sizeof(to))) < 0) {
    printf("Connect failed: %d (%s)\n", r, strerror(errno));
    return r;
  }
  
  printf("send\n");
  if ((r = send(sockfd, &buf, sizeof(buf), 0)) < 0) {
    return -1;
  }

  printf("recv\n");
  memset(&ret_buf[0], 0, sizeof(ret_buf));
  if ((r = recv(sockfd, &ret_buf[0], sizeof(ret_buf), 0)) < 0) {
    return r;
  }
  
  printf("close\n");
  close(sockfd);

  printf("ParseNaClHashResp\n");
  if ((r = ParseNaClHashResp(&ret_buf[0], sizeof(ret_buf), addr_in->sin_addr.s_addr, &remote_ports, 0)) != 0) {
    return r;
  }

  printf("verifyServerPort and return\n");
  return verifyServerPort(remote_ports, addr_in->sin_port);
}


/*Given a hash & nonce, fill in the buf with the message that needs to be sent to server*/
/*The buffer must have a length of 24 bytes.  The response will be 24 bytes long.*/
void MakeNaClHashReq(unsigned char *buf, unsigned char *hash, uint32_t nonce) {
  memcpy(buf, hash, 20);
  memcpy(buf+20, &nonce, 4);
}

/*Given the response from the server, fills in and sets ports to point to the NaClRemoteServerPorts struct*/
/*returns 0 on success nonzero on error*/
int ParseNaClHashResp(const char* buf, uint32_t buf_len, uint32_t server_ip, struct NaClRemoteServerPorts **ports, uint32_t nonce) {
  /* Format is as follows:
   * 
   * First 32 bits: success code (4-byte integer, in network order) -- 0 indicates success
   * Next 32 bits: Nonce that we sent to the server initially
   * Next 65536 bits: Bitmask of allowed ports
   */
  uint32_t *int_ptr;
  uint32_t success;
  uint32_t new_nonce;
  struct NaClRemoteServerPorts *p;

  printf("Allocated and UNREF\n");

  printf("Is buf_len is too small?\n");
  if (buf_len < 8200) {
    printf("Yep: %d < %d\n", buf_len, 8200);
    return -1; // We have to be able to parse out at least our three integers of interest
  }

  int_ptr = (uint32_t*)buf;
  success = ntohl(int_ptr[0]);
  new_nonce = ntohl(int_ptr[1]);

  if (success != 0) {
    printf("We fail: %d\n", success);
    return success;
  }

  if (new_nonce != nonce) {
    printf("Invalid nonce!  Failing...  (0x%x, 0x%x)\n", nonce, new_nonce);
    return -1;
  }

  printf("remoteServerPortsRingbuffer\n");
  p = &remoteServerPortsRingbuffer[rsprb_index++ % NUM_REMOTE_SERVER_PORTS];

  printf("Some memory tweakage\n");
  p->ip = server_ip;
  memcpy(&p->ports[0], buf + 8, 8192);

  *ports = p;

  printf("Done\n");
  return 0;
}

/*Given the filename of nexe, create a hash*/
/*"hash" must point to a 20-bytes buffer into which the hash will be written.*/
/*returns 0 on success nonzero on error*/
int MakeNaClHash(const char* nexe_file, unsigned char *hash) {
  int fd;
  struct stat filstat;
  int r;
  void *addr = 0;  

  if ((fd = open(nexe_file, O_RDONLY)) < 0) {
    return fd;
  }
  
  if ((r = fstat(fd, &filstat)) < 0) {
    return r;
  }

  addr = mmap(addr, filstat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

  SHA1((const unsigned char *)addr, filstat.st_size, hash);

  return 0;
}
