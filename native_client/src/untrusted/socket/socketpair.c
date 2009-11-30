//@author nizam
#include <errno.h>
#include <sys/types.h>
#include "native_client/src/trusted/service_runtime/include/sys/socket.h"
#include "native_client/src/untrusted/nacl/syscall_bindings_trampoline.h"

int socketpair(int domain, int type, int protocol, int fds[2]) {
	int retval = NACL_SYSCALL(socketpair)(domain, type, protocol, fds);
	if (retval < 0) {
	  errno = -retval;
	  return -1;
	}
	return retval;
}
