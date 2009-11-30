 //@author nizam
#include <errno.h>
#include <sys/types.h>
#include "native_client/src/trusted/service_runtime/include/sys/socket.h"
#include "native_client/src/untrusted/nacl/syscall_bindings_trampoline.h"

int getsockopt (int fd, int level, int optname,
		       void *optval,
		       socklen_t *optlen) {
  int retval = NACL_SYSCALL(getsockopt)(fd, level, optname, optval, optlen);
  if (retval < 0) {
	errno = -retval;
	return -1;
  }
  return retval;
}
