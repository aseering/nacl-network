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

#include "native_client/src/trusted/service_runtime/linux/nacl_socks_client.h"
#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"

#define NUM_REMOTE_SERVER_PORTS 20
struct NaClRemoteServerPorts remoteServerPortsRingbuffer[NUM_REMOTE_SERVER_PORTS];
int rsprb_index = 0;


int verifyServerPort(struct NaClRemoteServerPorts *p, short port, int protocol) {
  if (protocol == IPPROTO_TCP) {
    return ((p->tcp_ports[port/8] & 1 << (port % 8)) != 0) ? 0 : -1;
  } else if (protocol == IPPROTO_UDP) {
    return ((p->udp_ports[port/8] & 1 << (port % 8)) != 0) ? 0 : -1;
  } else {
    return (0 != 0);
  }
}


int NaClIsConnectionOk(const struct sockaddr *addr, unsigned char* hash) {
  // We've verified that this must (well, is supposed to) be an inet struct
  struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;

  int i, r;

  unsigned int recvfrom_len;

  unsigned char buf[24];
  char ret_buf[1024];
  char itoa_buf[33];
  int len;
  struct NaClRemoteServerPorts *remote_ports;

  struct addrinfo hints;
  struct addrinfo *servinfo;
  struct sockaddr from;
  int sockfd;


  if (addr->sa_family != AF_INET) {
    return -1; // We don't support anything != IPv4 at this time
  }

  // See if we already know what to do with this IP
  for (i = 0; i < NUM_REMOTE_SERVER_PORTS; i++) {
    if (remoteServerPortsRingbuffer[i].ip == addr_in->sin_addr.s_addr) {
      return verifyServerPort(&remoteServerPortsRingbuffer[i], addr_in->sin_port, IPPROTO_TCP);
    }
  }

  // We don't.  So, go fetch a response, and use it.
  // Somehow, we have to get our own hash.
  // For now, just hardcode a magic value.
  
  MakeNaClHashReq(&buf[0], hash, 0);
  
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  
  sprintf(&itoa_buf[0], "%d", NACL_VALIDATE_SERVERPORT);
  if ((r = getaddrinfo(NULL, &itoa_buf[0], &hints, &servinfo)) != 0) {
    return -1;
  }
  
  if ((sockfd = socket(servinfo[0].ai_family, servinfo[0].ai_socktype, servinfo[0].ai_protocol)) == -1)  {
    return sockfd;
  }
  
  sendto(sockfd, &buf, sizeof(buf), 0, addr, sizeof(struct sockaddr_storage));
  recvfrom_len = sizeof(struct sockaddr);
  if ((len = recvfrom(sockfd, &ret_buf, sizeof(ret_buf), 0, &from, &recvfrom_len)) < 0) {
    return -1;
  }

  close(sockfd);

  if ((r = ParseNaClHashResp(&ret_buf[0], len, addr_in->sin_addr.s_addr, &remote_ports, 0)) != 0) {
    return r;
  }

  // hm...  We don't know here whether this request is for UDP or TCP (/etc).
  // For now, assume TCP.
  return verifyServerPort(remote_ports, addr_in->sin_port, IPPROTO_TCP);
}


/*Given a hash & nonce, fill in the buf with the message that needs to be sent to server*/
/*The buffer must have a length of 24 bytes.  The response will be 24 bytes long.*/
void MakeNaClHashReq(unsigned char *buf, unsigned char *hash, int nonce) {
  memcpy(buf, hash, 20);
  memcpy(buf+20, &nonce, 4);
}

/*Given the response from the server, fills in and sets ports to point to the NaClRemoteServerPorts struct*/
/*returns 0 on success nonzero on error*/
int ParseNaClHashResp(const char* buf, int buf_len, unsigned int server_ip, struct NaClRemoteServerPorts **ports, int nonce) {
  /* Format is as follows:
   * 
   * First 32 bits: success code (4-byte integer, in network order) -- 0 indicates success
   * Next 65536 bits: Bitmask of allowed TCP ports
   * Next 65535 bits: Bitmask of allowed UDP ports
   */
  int *int_ptr;
  int success;
  struct NaClRemoteServerPorts *p;

  UNREFERENCED_PARAMETER(ports);
  UNREFERENCED_PARAMETER(nonce);

  if (buf_len < 4 + 2 * 8192) {
    return -1; // We have to be able to parse out at least our three integers of interest
  }

  int_ptr = (int*)buf;
  success = int_ptr[0];

  if (success != 0) {
    return success;
  }

  p = &remoteServerPortsRingbuffer[rsprb_index++ % NUM_REMOTE_SERVER_PORTS];

  p->ip = server_ip;
  memcpy(&p->tcp_ports, buf + 4, 8192);
  memcpy(&p->udp_ports, buf + 4 + 8192, 8192);

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
