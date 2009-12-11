#include <openssl/sha.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>

/*Parse the request coming from the client*/
/*0 on success*/
int ParseNaClHashReq(const char* buf, uint32_t* nonce, unsigned char hash[20]) {
  memcpy(&hash[0], buf, 20);
  memcpy(nonce, buf+20, 4);
  return 0;
}

/*make a response*/
void MakeNaClHashResp(char* buf, const uint32_t* ports, uint32_t nonce, uint32_t ok) {
  int i;
  char *ports_map;

  uint32_t *int_ptr = (uint32_t *)buf;
  int_ptr[0] = htonl(ok);
  int_ptr[1] = nonce;// Don't bother transposing; we never un-transposed

  memset(&int_ptr[2], 0, 8192); // The tcpports and udpports buffers

  i = 0;
  ports_map = buf + 8;
  while (ports[i] != 0) {
    ports_map[ports[i]/8] |= (1 << (ports[i]%8));
    i++;
  }
}

/*optional: create and return hash given the data of nexe, developer can use if her nexes are really small*/
/*If "nexeData" points to an mmap'ed file, this function should return the same hash as would running "sha1sum" on that file.*/
void MakeNaClHash(const void* nexeData, int nexeDataLength, unsigned char hash[20]) {
  SHA1(nexeData, nexeDataLength, &hash[0]);
}
