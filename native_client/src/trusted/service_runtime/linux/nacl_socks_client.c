#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "native_client/src/trusted/service_runtime/linux/nacl_socket_client.h"


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


int NaClIsConnectionOk(int ip, short port, int domain, int type, int protocol) {
  if (domain != PF_INET && domain != PF_INET6) {
    return -1; // We don't support anything != IPv4 or IPv6 at this time
  }

  // We support any type of connection at this time;
  // though we may need to filter that in the future,
  // so this API accepts and ignores 'type'.

  if (protocol != IPPROTO_TCP && protocol != IPPROTO_UDP) {
    return -1; // We only support TCP and UDP right now
  }

  // See if we already know what to do with this IP
  int i;
  for (i = 0; i < NUM_REMOTE_SERVER_PORTS; i++) {
    if (remoteServerPortsRingBuffer[i].ip == ip) {
      return verifyServerPort(&remoteServerPortsRingbuffer[i], port, protocol);
    }
  }

  // We don't.  So, go fetch a response, and use it.
  /* TODO */
}


/*Given a hash & nonce, fill in the buf with the message that needs to be sent to server*/
/*The buffer must have a length of 24 bytes.  The response will be 24 bytes long.*/
void MakeNaClHashReq(unsigned char *buf, unsigned char *hash, int nonce) {
  memcpy(buf, hash, 20);
  memcpy(buf+20, &nonce, 4);
}

/*Given the response from the server, fills in and sets ports to point to the NaClRemoteServerPorts struct*/
/*returns 0 on success nonzero on error*/
int ParseNaClHashResp(const char* buf, int buf_len, int server_ip, struct NaClRemoteServerPorts **ports) {
  /* Format is as follows:
   * 
   * First 32 bits: success code (4-byte integer, in network order) -- 0 indicates success
   * Next 65536 bits: Bitmask of allowed TCP ports
   * Next 65535 bits: Bitmask of allowed UDP ports
   */
  if (buf_len < 4 + 2 * 8192) {
    return -1; // We have to be able to parse out at least our three integers of interest
  }

  int *int_ptr;

  int success;

  int_ptr = buf;
  success = inet_ntop(int_ptr[0]);

  if (success != 0) {
    return success;
  }

  struct NaClRemoteServerPorts *p;
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

  if ((fd = open(nexe_file, O_RDONLY)) < 0) {
    return fd;
  }
  
  struct stat filstat;
  int r;

  if ((r = fstat(fd, &filstat)) < 0) {
    return r;
  }

  void *addr = 0;  
  if ((addr = mmap(addr, filstat.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) < 0) {
    close(fd);
    return (int)addr;
  }

  if ((r = SHA1(addr, filstat.st_size, hash)) < 0) {
    return r;
  }

  return 0;
}
