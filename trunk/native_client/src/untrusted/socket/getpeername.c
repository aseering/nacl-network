 //@author nizam
#include <errno.h>
#include <sys/types.h>
#include "native_client/src/trusted/service_runtime/include/sys/socket.h"
#include "native_client/src/untrusted/nacl/syscall_bindings_trampoline.h"

int getpeername (int fd, struct sockaddr* addr, socklen_t *len) {
	int retval = NACL_SYSCALL(getpeername)(fd, addr, len);
	if (retval < 0) {
		errno = -retval;
		return -1;
	}
	return retval;
}
