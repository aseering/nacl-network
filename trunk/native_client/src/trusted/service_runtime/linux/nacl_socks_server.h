#ifndef NATIVE_CLIENT_SERVICE_RUNTIME_LINUX_NACL_SOCKS_SERVER_H_
#define NATIVE_CLIENT_SERVICE_RUNTIME_LINUX_NACL_SOCKS_SERVER_H_
/*Parse the request coming from the client*/
/*0 on success*/
int ParseNaClHashReq(const char* buf, int* nonce, unsigned int hash[20]);
/*make a response*/
void MakeNaclResp(char* buf, const int* tcpports, const int* udpports, int nonce, int global, int ok);
/*optional: create and return hash given the data of nexe, developer can use if her nexes are really small*/
void MakeHash(const void* nexeData, unsigned int hash[20]);
#endif
