//@author nizam
#ifndef NATIVE_CLIENT_SERVICE_RUNTIME_INCLUDE_BITS_SOCKADDR_H_
#define NATIVE_CLIENT_SERVICE_RUNTIME_INCLUDE_BITS_SOCKADDR_H_



/* POSIX.1g specifies this type name for the `sa_family' member.  */
typedef unsigned short int sa_family_t;

/* This macro is used to declare the initial common members
   of the data types used for socket addresses, `struct sockaddr',
   `struct sockaddr_in', `struct sockaddr_un', etc.  */

#define	__SOCKADDR_COMMON(sa_prefix) \
  sa_family_t sa_prefix##family

#define __SOCKADDR_COMMON_SIZE	(sizeof (unsigned short int))

#endif	/* NATIVE_CLIENT_SERVICE_RUNTIME_INCLUDE_BITS_SOCKADDR_H_ */
