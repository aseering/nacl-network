/*
 * Copyright 2008, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#if !defined(ARRAYSIZE)
#define ARRAYSIZE(a) sizeof(a)/sizeof(a[0])
#endif


/* 32-bit system call table */
#ifdef __i386__
static const char* syscall_data_i386[] = {
  "restart_syscall",
  "exit",
  "fork",
  "read",
  "write",
  "open",
  "close",
  "waitpid",
  "creat",
  "link",
  "unlink",
  "execve",
  "chdir",
  "time",
  "mknod",
  "chmod",
  "lchown",
  "break",
  "oldstat",
  "lseek",
  "getpid",
  "mount",
  "umount",
  "setuid",
  "getuid",
  "stime",
  "ptrace",
  "alarm",
  "oldfstat",
  "pause",
  "utime",
  "stty",
  "gtty",
  "access",
  "nice",
  "ftime",
  "sync",
  "kill",
  "rename",
  "mkdir",
  "rmdir",
  "dup",
  "pipe",
  "times",
  "prof",
  "brk",
  "setgid",
  "getgid",
  "signal",
  "geteuid",
  "getegid",
  "acct",
  "umount2",
  "lock",
  "ioctl",
  "fcntl",
  "mpx",
  "setpgid",
  "ulimit",
  "oldolduname",
  "umask",
  "chroot",
  "ustat",
  "dup2",
  "getppid",
  "getpgrp",
  "setsid",
  "sigaction",
  "sgetmask",
  "ssetmask",
  "setreuid",
  "setregid",
  "sigsuspend",
  "sigpending",
  "sethostname",
  "setrlimit",
  "getrlimit",
  "getrusage",
  "gettimeofday",
  "settimeofday",
  "getgroups",
  "setgroups",
  "select",
  "symlink",
  "oldlstat",
  "readlink",
  "uselib",
  "swapon",
  "reboot",
  "readdir",
  "mmap",
  "munmap",
  "truncate",
  "ftruncate",
  "fchmod",
  "fchown",
  "getpriority",
  "setpriority",
  "profil",
  "statfs",
  "fstatfs",
  "ioperm",
  "socketcall",
  "syslog",
  "setitimer",
  "getitimer",
  "stat",
  "lstat",
  "fstat",
  "olduname",
  "iopl",
  "vhangup",
  "idle",
  "vm86old",
  "wait4",
  "swapoff",
  "sysinfo",
  "ipc",
  "fsync",
  "sigreturn",
  "clone",
  "setdomainname",
  "uname",
  "modify_ldt",
  "adjtimex",
  "mprotect",
  "sigprocmask",
  "create_module",
  "init_module",
  "delete_module",
  "get_kernel_syms",
  "quotactl",
  "getpgid",
  "fchdir",
  "bdflush",
  "sysfs",
  "personality",
  "afs_syscall",
  "setfsuid",
  "setfsgid",
  "_llseek",
  "getdents",
  "_newselect",
  "flock",
  "msync",
  "readv",
  "writev",
  "getsid",
  "fdatasync",
  "_sysctl",
  "mlock",
  "munlock",
  "mlockall",
  "munlockall",
  "sched_setparam",
  "sched_getparam",
  "sched_setscheduler",
  "sched_getscheduler",
  "sched_yield",
  "sched_get_priority_max",
  "sched_get_priority_min",
  "sched_rr_get_interval",
  "nanosleep",
  "mremap",
  "setresuid",
  "getresuid",
  "vm86",
  "query_module",
  "poll",
  "nfsservctl",
  "setresgid",
  "getresgid",
  "prctl",
  "rt_sigreturn",
  "rt_sigaction",
  "rt_sigprocmask",
  "rt_sigpending",
  "rt_sigtimedwait",
  "rt_sigqueueinfo",
  "rt_sigsuspend",
  "pread64",
  "pwrite64",
  "chown",
  "getcwd",
  "capget",
  "capset",
  "sigaltstack",
  "sendfile",
  "getpmsg",
  "putpmsg",
  "vfork",
  "ugetrlimit",
  "mmap2",
  "truncate64",
  "ftruncate64",
  "stat64",
  "lstat64",
  "fstat64",
  "lchown32",
  "getuid32",
  "getgid32",
  "geteuid32",
  "getegid32",
  "setreuid32",
  "setregid32",
  "getgroups32",
  "setgroups32",
  "fchown32",
  "setresuid32",
  "getresuid32",
  "setresgid32",
  "getresgid32",
  "chown32",
  "setuid32",
  "setgid32",
  "setfsuid32",
  "setfsgid32",
  "pivot_root",
  "mincore",
  "madvise",
  "getdents64",
  "fcntl64",
  "unused_222",
  "unused_223",
  "gettid",
  "readahead",
  "setxattr",
  "lsetxattr",
  "fsetxattr",
  "getxattr",
  "lgetxattr",
  "fgetxattr",
  "listxattr",
  "llistxattr",
  "flistxattr",
  "removexattr",
  "lremovexattr",
  "fremovexattr",
  "tkill",
  "sendfile64",
  "futex",
  "sched_setaffinity",
  "sched_getaffinity",
  "set_thread_area",
  "get_thread_area",
  "io_setup",
  "io_destroy",
  "io_getevents",
  "io_submit",
  "io_cancel",
  "fadvise64",
  "unused_251",
  "exit_group",
  "lookup_dcookie",
  "epoll_create",
  "epoll_ctl",
  "epoll_wait",
  "remap_file_pages",
  "set_tid_address",
  "timer_create",
  "timer_settime",
  "timer_gettime",
  "timer_getoverrun",
  "timer_delete",
  "clock_settime",
  "clock_gettime",
  "clock_getres",
  "clock_nanosleep",
  "statfs64",
  "fstatfs64",
  "tgkill",
  "utimes",
  "fadvise64_64",
  "vserver",
  "mbind",
  "get_mempolicy",
  "set_mempolicy",
};

#else
/* 64-bit system call table */
static const char* syscall_data_x86_64[] = {
  "read",
  "write",
  "open",
  "close",
  "stat",
  "fstat",
  "lstat",
  "poll",
  "lseek",
  "mmap",
  "mprotect",
  "munmap",
  "brk",
  "rt_sigaction",
  "rt_sigprocmask",
  "rt_sigreturn",
  "ioctl",
  "pread64",
  "pwrite64",
  "readv",
  "writev",
  "access",
  "pipe",
  "select",
  "sched_yield",
  "mremap",
  "msync",
  "mincore",
  "madvise",
  "shmget",
  "shmat",
  "shmctl",
  "dup",
  "dup2",
  "pause",
  "nanosleep",
  "getitimer",
  "alarm",
  "setitimer",
  "getpid",
  "sendfile",
  "socket",
  "connect",
  "accept",
  "sendto",
  "recvfrom",
  "sendmsg",
  "recvmsg",
  "shutdown",
  "bind",
  "listen",
  "getsockname",
  "getpeername",
  "socketpair",
  "setsockopt",
  "getsockopt",
  "clone",
  "fork",
  "vfork",
  "execve",
  "exit",
  "wait4",
  "kill",
  "uname",
  "semget",
  "semop",
  "semctl",
  "shmdt",
  "msgget",
  "msgsnd",
  "msgrcv",
  "msgctl",
  "fcntl",
  "flock",
  "fsync",
  "fdatasync",
  "truncate",
  "ftruncate",
  "getdents",
  "getcwd",
  "chdir",
  "fchdir",
  "rename",
  "mkdir",
  "rmdir",
  "creat",
  "link",
  "unlink",
  "symlink",
  "readlink",
  "chmod",
  "fchmod",
  "chown",
  "fchown",
  "lchown",
  "umask",
  "gettimeofday",
  "getrlimit",
  "getrusage",
  "sysinfo",
  "times",
  "ptrace",
  "getuid",
  "syslog",
  "getgid",
  "setuid",
  "setgid",
  "geteuid",
  "getegid",
  "setpgid",
  "getppid",
  "getpgrp",
  "setsid",
  "setreuid",
  "setregid",
  "getgroups",
  "setgroups",
  "setresuid",
  "getresuid",
  "setresgid",
  "getresgid",
  "getpgid",
  "setfsuid",
  "setfsgid",
  "getsid",
  "capget",
  "capset",
  "rt_sigpending",
  "rt_sigtimedwait",
  "rt_sigqueueinfo",
  "rt_sigsuspend",
  "sigaltstack",
  "utime",
  "mknod",
  "uselib",
  "personality",
  "ustat",
  "statfs",
  "fstatfs",
  "sysfs",
  "getpriority",
  "setpriority",
  "sched_setparam",
  "sched_getparam",
  "sched_setscheduler",
  "sched_getscheduler",
  "sched_get_priority_max",
  "sched_get_priority_min",
  "sched_rr_get_interval",
  "mlock",
  "munlock",
  "mlockall",
  "munlockall",
  "vhangup",
  "modify_ldt",
  "pivot_root",
  "_sysctl",
  "prctl",
  "arch_prctl",
  "adjtimex",
  "setrlimit",
  "chroot",
  "sync",
  "acct",
  "settimeofday",
  "mount",
  "umount2",
  "swapon",
  "swapoff",
  "reboot",
  "sethostname",
  "setdomainname",
  "iopl",
  "ioperm",
  "create_module",
  "init_module",
  "delete_module",
  "get_kernel_syms",
  "query_module",
  "quotactl",
  "nfsservctl",
  "getpmsg",
  "putpmsg",
  "afs_syscall",
  "tuxcall",
  "security",
  "gettid",
  "readahead",
  "setxattr",
  "lsetxattr",
  "fsetxattr",
  "getxattr",
  "lgetxattr",
  "fgetxattr",
  "listxattr",
  "llistxattr",
  "flistxattr",
  "removexattr",
  "lremovexattr",
  "fremovexattr",
  "tkill",
  "time",
  "futex",
  "sched_setaffinity",
  "sched_getaffinity",
  "set_thread_area",
  "io_setup",
  "io_destroy",
  "io_getevents",
  "io_submit",
  "io_cancel",
  "get_thread_area",
  "lookup_dcookie",
  "epoll_create",
  "epoll_ctl_old",
  "epoll_wait_old",
  "remap_file_pages",
  "getdents64",
  "set_tid_address",
  "restart_syscall",
  "semtimedop",
  "fadvise64",
  "timer_create",
  "timer_settime",
  "timer_gettime",
  "timer_getoverrun",
  "timer_delete",
  "clock_settime",
  "clock_gettime",
  "clock_getres",
  "clock_nanosleep",
  "exit_group",
  "epoll_wait",
  "epoll_ctl",
  "tgkill",
  "utimes",
  "vserver",
  "mbind",
  "set_mempolicy",
  "get_mempolicy",
};
#endif

const char* GetSyscallName(int no) {
  unsigned uno = (unsigned) no;
#ifdef __i386__
  if (ARRAYSIZE(syscall_data_i386) <= uno) {
    return "@INVALID-SYSCALL-NAME";
  }

  return  syscall_data_i386[uno];
#else
  if (ARRAYSIZE(syscall_data_x86_64) <= uno) {
    return "@INVALID-SYSCALL-NAME";
  }

  return  syscall_data_x86_64[uno];
#endif
}
