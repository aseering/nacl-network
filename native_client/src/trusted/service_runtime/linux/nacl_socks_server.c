#include <openssl/sha.h>

/*Parse the request coming from the client*/
/*0 on success*/
int ParseNaClHashReq(const char* buf, int* nonce, unsigned int hash[20]) {
  memcpy(&hash, buf, 20);
  memcpy(nonce, buf+20, 4);
}

/*make a response*/
void MakeNaClHashResp(char* buf, const int* tcpports, const int* udpports, int nonce, int global, int ok) {
  memcpy(buf, &ok, 4);
  memset(buf + 4, 0, 16384); // The tcpports and udpports buffers

  int i;
  unsigned char *ports;

  // tcpports
  i = 0;
  ports = buf + 4;
  while (tcpports[i] != 0) {
    ports[tcpports[i]/8] |= 1 << (tcpports[i]%8);
  }


  // udpports
  i = 0;
  ports = buf + 4 + 8192;
  while (tcpports[i] != 0) {
    ports[udpports[i]/8] |= 1 << (udpports[i]%8);
  }
}

/*optional: create and return hash given the data of nexe, developer can use if her nexes are really small*/
/*If "nexeData" points to an mmap'ed file, this function should return the same hash as would running "sha1sum" on that file.*/
void MakeNaClHash(const void* nexeData, int nexeDataLength, unsigned char hash[20]) {
  SHA1(nexeData, nexeDataLength, &hash);
}
