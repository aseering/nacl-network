#ifndef NATIVE_CLIENT_SERVICE_RUNTIME_LINUX_NACL_SOCKS_SERVER_H_
#define NATIVE_CLIENT_SERVICE_RUNTIME_LINUX_NACL_SOCKS_SERVER_H_
/*Parse the request coming from the client*/
/*0 on success*/
int ParseNaClHashReq(const char* buf, uint32_t* nonce, unsigned char hash[20]);
/*make a response*/
void MakeNaClHashResp(char* buf, const uint32_t* ports, uint32_t nonce, uint32_t ok);
/*optional: create and return hash given the data of nexe, developer can use if her nexes are really small*/
void MakeNaClHash(const void* nexeData, int nexeDataLength, unsigned int hash[20]);
#endif
