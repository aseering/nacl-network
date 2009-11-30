 //@author nizam
#include <errno.h>
#include <sys/types.h>
#include "native_client/src/trusted/service_runtime/include/sys/socket.h"
#include "native_client/src/untrusted/nacl/syscall_bindings_trampoline.h"

int accept (int fd, struct sockaddr* addr,
		   socklen_t *addr_len) {
  int retval = NACL_SYSCALL(accept)(fd, addr, addr_len);
  if (retval < 0) {
		errno = -retval;
		return -1;
  }
  return retval;
}
