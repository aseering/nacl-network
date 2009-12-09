#include <sys/socket.h>

#ifndef NATIVE_CLIENT_SERVICE_RUNTIME_LINUX_NACL_SOCKS_CLIENT_H_
#define NATIVE_CLIENT_SERVICE_RUNTIME_LINUX_NACL_SOCKS_CLIENT_H_

#define NACL_VALIDATE_SERVERPORT 1123

/*Struct indicating what ports on a given IP address we are allowed to connect to*/
struct NaClRemoteServerPorts {
  unsigned int ip;
  unsigned char udp_ports[8192];
  unsigned char tcp_ports[8192];
};

/*Given a hash & nonce, fill in the buf with the message that needs to be sent to server*/
void MakeNaClHashReq(unsigned char *buf, unsigned char *hash, int nonce);

/*Given the response from the server, fills in udpPorts, tcpPorts, sets global and nonce*/
/*returns 0 on success nonzero on error*/
int ParseNaClHashResp(const char* buf, int buf_len, unsigned int server_ip, struct NaClRemoteServerPorts **ports, int nonce);

/*Given the data of nexe, create a hash*/
/*"hash" must point to a 20-bytes buffer into which the hash will be written.*/
/*returns 0 on success nonzero on error*/
int MakeNaClHash(const char* nexe_file, unsigned char *hash);

/*Given a 'struct sockaddr' object, determine if it's making a valid connection.*/
/*returns 0 if it would be ok to make this connection; nonzero on error (including permissiond failure).*/
/*This function may open a connection to the specified server in order to determine permissions.*/
int NaClIsConnectionOk(const struct sockaddr *addr, unsigned char *hash);

#endif
