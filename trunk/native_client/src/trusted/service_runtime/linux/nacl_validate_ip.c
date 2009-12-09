#include <sys/socket.h>

#include "native_client/src/include/portability.h"

#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"
#include "native_client/src/trusted/service_runtime/linux/nacl_socks_client.h"

#define IGNORE_CHECKS
#define SERVER_PORT 11234

int NaClValidateIp(struct NaClAppThread  *natp, const struct sockaddr* addr) {
#ifdef IGNORE_CHECKS
	UNREFERENCED_PARAMETER(natp);
	UNREFERENCED_PARAMETER(addr);
	return 0;
#else
	int i;
	int r;
	int sockfd;

	// Allow privileged apps automatically
	if (natp->is_privileged) {
		return 0;
	}

	return NaClIsConnectionOk(addr);
	
#endif // IGNORE_CHECKS
}
