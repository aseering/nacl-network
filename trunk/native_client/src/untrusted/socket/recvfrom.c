 //@author nizam
#include <errno.h>
#include <sys/types.h>
#include "native_client/src/trusted/service_runtime/include/sys/socket.h"
#include "native_client/src/untrusted/nacl/syscall_bindings_trampoline.h"

ssize_t recvfrom (int fd, void *buf, size_t n,
			 int flags, struct sockaddr* addr,
			 socklen_t *addr_len) {
  int retval = NACL_SYSCALL(recvfrom)(fd, buf, n, flags, addr, addr_len);
  if (retval < 0) {
		errno = -retval;
		return -1;
	}
	return retval;
}
