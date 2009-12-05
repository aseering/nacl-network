#include <sys/socket.h>

#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"

#define IGNORE_CHECKS

int NaClValidateIp(struct NaClAppThread  *natp, struct sockaddr* addr) {
#ifdef IGNORE_CHECKS
  return 0;
#else
  // Allow privileged apps automatically
  if (natp->is_privileged) {
    return 0;
  }

  // Somehow, we have to get our own hash.
  // For now, just hardcode a magic value.
  int 


  
  return -1;
#endif // IGNORE_CHECKS
}
