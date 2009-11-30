//@author: nizam
#ifndef NATIVE_CLIENT_SERVICE_RUNTIME_INCLUDE_SYS_SOCKET_H_
#define NATIVE_CLIENT_SERVICE_RUNTIME_INCLUDE_SYS_SOCKET_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <bits/socket.h>
/* The following constants should be used for the second parameter of
   `shutdown'.  */


#define SHUT_RD		0

#define SHUT_WR		1
/* No more receptions or transmissions.  */
#define SHUT_RDWR	2







/* Create a new socket of type TYPE in domain DOMAIN, using
   protocol PROTOCOL.  If PROTOCOL is zero, one is chosen automatically.
   Returns a file descriptor for the new socket, or -1 for errors.  */
extern int socket (int domain, int type, int protocol);

/* Create two new sockets, of type TYPE in domain DOMAIN and using
   protocol PROTOCOL, which are connected to each other, and put file
   descriptors for them in FDS[0] and FDS[1].  If PROTOCOL is zero,
   one will be chosen automatically.  Returns 0 on success, -1 for errors.  */
extern int socketpair (int domain, int type, int protocol,
		       int fds[2]);

/* Give the socket FD the local address ADDR (which is LEN bytes long).  */
extern int bind (int fd, const struct sockaddr * addr, unsigned int len);

/* Put the local address of FD into *ADDR and its length in *LEN.  */
extern int getsockname (int fd, struct sockaddr * addr,
			unsigned int *len);

/* Open a connection on socket FD to peer at ADDR (which LEN bytes long).
   For connectionless socket types, just set the default address to send to
   and the only address from which to accept transmissions.
   Return 0 on success, -1 for errors.

   This function is a cancellation point and therefore not marked with
   THROW.  */
extern int connect (int fd, const struct sockaddr * addr, unsigned int len);

/* Put the address of the peer connected to socket FD into *ADDR
   (which is *LEN bytes long), and its actual length into *LEN.  */
extern int getpeername (int fd, struct sockaddr * addr,
			unsigned int *len);


/* Send N bytes of BUF to socket FD.  Returns the number sent or -1.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t send (int fd, const void *buf, size_t n, int flags);

/* Read N bytes into BUF from socket FD.
   Returns the number read or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t recv (int fd, void *buf, size_t n, int flags);

/* Send N bytes of BUF on socket FD to peer at address ADDR (which is
   ADDR_LEN bytes long).  Returns the number sent, or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t sendto (int fd, const void *buf, size_t n,
		       int flags, const struct sockaddr * addr,
		       unsigned int addr_len);

/* Read N bytes into BUF through socket FD.
   If ADDR is not NULL, fill in *ADDR_LEN bytes of it with tha address of
   the sender, and store the actual size of the address in *ADDR_LEN.
   Returns the number of bytes read or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t recvfrom (int fd, void *buf, size_t n,
			 int flags, struct sockaddr * addr,
			 unsigned int *addr_len);


/* Send a message described MESSAGE on socket FD.
   Returns the number of bytes sent, or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t sendmsg (int fd, const struct msghdr *message,
			int flags);

/* Receive a message as described by MESSAGE from socket FD.
   Returns the number of bytes read or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t recvmsg (int fd, struct msghdr *message, int flags);


/* Put the current value for socket FD's option OPTNAME at protocol level LEVEL
   into OPTVAL (which is *OPTLEN bytes long), and set *OPTLEN to the value's
   actual length.  Returns 0 on success, -1 for errors.  */
extern int getsockopt (int fd, int level, int optname,
		       void * optval,
		       unsigned int * optlen);

/* Set socket FD's option OPTNAME at protocol level LEVEL
   to *OPTVAL (which is OPTLEN bytes long).
   Returns 0 on success, -1 for errors.  */
extern int setsockopt (int fd, int level, int optname,
		       const void *optval, unsigned int optlen);


/* Prepare to accept connections on socket FD.
   N connection requests will be queued before further requests are refused.
   Returns 0 on success, -1 for errors.  */
extern int listen (int fd, int n);

/* Await a connection on socket FD.
   When a connection arrives, open a new socket to communicate with it,
   set *ADDR (which is *ADDR_LEN bytes long) to the address of the connecting
   peer and *ADDR_LEN to the address's actual length, and return the
   new socket's descriptor, or -1 for errors.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern int accept (int fd, struct sockaddr * addr,
		   unsigned int *addr_len);

/* Shut down all or part of the connection open on socket FD.
   HOW determines what to shut down:
     SHUT_RD   = No more receptions;
     SHUT_WR   = No more transmissions;
     SHUT_RDWR = No more receptions or transmissions.
   Returns 0 on success, -1 for errors.  */
extern int shutdown (int fd, int how);

#ifdef __cplusplus
}
#endif


#endif /* NATIVE_CLIENT_SERVICE_RUNTIME_INCLUDE_SYS_SOCKET_H_ */
