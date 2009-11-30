 //@author nizam
#include <errno.h>
#include <sys/types.h>
#include "native_client/src/trusted/service_runtime/include/sys/socket.h"
#include "native_client/src/untrusted/nacl/syscall_bindings_trampoline.h"

ssize_t recv (int fd, void *buf, size_t n, int flags) {
  int retval = NACL_SYSCALL(recv)(fd, buf, n, flags);
  if (retval < 0) {
	errno = -retval;
	return -1;
  }
  return retval;
}
