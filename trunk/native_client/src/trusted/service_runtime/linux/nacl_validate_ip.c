#include <sys/socket.h>

#include "native_client/src/include/portability.h"

#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"
#include "native_client/src/trusted/service_runtime/linux/nacl_socks_client.h"

//#define IGNORE_CHECKS
//#define ALLOW_PRIVILEGED_FULL_NETWORK
#define SERVER_PORT 11234

int NaClValidateIp(struct NaClAppThread *natp, const struct sockaddr* addr) {
#ifdef IGNORE_CHECKS
	UNREFERENCED_PARAMETER(natp);
	UNREFERENCED_PARAMETER(addr);
	return 0;
#else
	
#ifdef ALLOW_PRIVILEGED_FULL_NETWORK
	// Allow privileged apps automatically	
	if (natp->is_privileged) {
		return 0;
	}
#endif // ALLOW_PRIVILEGED_FULL_NETWORK

	return NaClIsConnectionOk(addr, &(natp->nap->app_hash[0]));	
#endif // IGNORE_CHECKS
}
