#ifndef NATIVE_CLIENT_SERVICE_RUNTIME_LINUX_NACL_SOCKS_CLIENT_H_
#define NATIVE_CLIENT_SERVICE_RUNTIME_LINUX_NACL_SOCKS_CLIENT_H_

/*Given a hash & nonce, fill in the buf with the message that needs to be sent to server*/
void MakeNaClHashReq(char *buf, unsigned int hash[20], int nonce);

/*Given the response from the server, fills in udpPorts, tcpPorts, sets global and nonce*/
/*returns 0 on success nonzero on error*/
int ParseNaClHashResp(const char* buf, int* udpPorts, int* tcpPorts, int* global, int *nonce);

#endif
