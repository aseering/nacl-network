//@author nizam
#include <errno.h>
#include <sys/types.h>
#include "native_client/src/trusted/service_runtime/include/sys/socket.h"
#include "native_client/src/untrusted/nacl/syscall_bindings_trampoline.h"

int socket (int domain, int type, int protocol) {
	int retval = NACL_SYSCALL(socket)(domain, type, protocol);
	if (retval < 0) {
	  errno = -retval;
	  return -1;
	}
	return retval;
}
