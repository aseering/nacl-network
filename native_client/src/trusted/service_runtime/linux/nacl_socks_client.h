#ifndef NATIVE_CLIENT_SERVICE_RUNTIME_LINUX_NACL_SOCKS_CLIENT_H_
#define NATIVE_CLIENT_SERVICE_RUNTIME_LINUX_NACL_SOCKS_CLIENT_H_

/*Given a hash & nonce, fill in the buf with the message that needs to be sent to server*/
void MakeNaClHashReq(char *buf, unsigned int hash[20], int nonce);

/*Given the response from the server, fills in udpPorts, tcpPorts, sets global and nonce*/
/*returns 0 on success nonzero on error*/
int ParseNaClHashResp(const char* buf, int* udpPorts, int* tcpPorts, int* global, int *nonce);

/*Given the data of nexe, create a hash*/
/*"hash" must point to a 20-bytes buffer into which the hash will be written.*/
/*returns 0 on success nonzero on error*/
int MakeNaClHash(const char* nexe_file, unsigned char *hash);


struct NaClRemoteServerPorts {
  int ip;
  unsigned char[8192] udp_ports;
  unsigned char[8192] tcp_ports;
};

#endif
