#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <asm/stat.h>
#include <errno.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <poll.h>

#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "x64run.h"
#include "x64emu_private.h"
#include "x64run_private.h"
//#include "x64primop.h"
#include "x64trace.h"
//#include "myalign.h"
#include "box64context.h"
#include "callback.h"
//#include "signals.h"
#include "x64tls.h"

typedef struct x64_sigaction_s x64_sigaction_t;
typedef struct x64_stack_s x64_stack_t;

int mkdir(const char *path, mode_t mode);

//int32_t my_getrandom(x64emu_t* emu, void* buf, uint32_t buflen, uint32_t flags);
int of_convert(int flag);
int32_t my_open(x64emu_t* emu, void* pathname, int32_t flags, uint32_t mode);
ssize_t my_readlink(x64emu_t* emu, void* path, void* buf, size_t sz);
int my_stat(x64emu_t *emu, void* filename, void* buf);

int my_sigaction(x64emu_t* emu, int signum, const x64_sigaction_t *act, x64_sigaction_t *oldact);
int my_sigaltstack(x64emu_t* emu, const x64_stack_t* ss, x64_stack_t* oss);
//int32_t my_execve(x64emu_t* emu, const char* path, char* const argv[], char* const envp[]);
void* my_mmap64(x64emu_t* emu, void *addr, unsigned long length, int prot, int flags, int fd, int64_t offset);
int my_munmap(x64emu_t* emu, void* addr, unsigned long length);
int my_mprotect(x64emu_t* emu, void *addr, unsigned long len, int prot);
void* my_mremap(x64emu_t* emu, void* old_addr, size_t old_size, size_t new_size, int flags, void* new_addr);

// cannot include <fcntl.h>, it conflict with some asm includes...
#ifndef O_NONBLOCK
#define O_NONBLOCK 04000
#endif
#undef fcntl
int fcntl(int fd, int cmd, ... /* arg */ );

// Syscall table for x86_64 can be found 
typedef struct scwrap_s {
    uint32_t x64s; // 32 bits?
    int nats;
    int nbpars;
} scwrap_t;

scwrap_t syscallwrap[] = {
    //{ 0, __NR_read, 3 },      // wrapped so SA_RESTART can be handled by libc
    //{ 1, __NR_write, 3 },     // same
    //{ 2, __NR_open, 3 },      // flags need transformation
    //{ 3, __NR_close, 1 },     // wrapped so SA_RESTART can be handled by libc
    //{ 4, __NR_stat, 2 },     // Need to align struct stat
    //{ 9, __NR_mmap, 6},       // wrapped to track mmap
    //{ 10, __NR_mprotect, 3},  // same
    //{ 11, __NR_munmap, 2},    // same
    { 5, __NR_fstat, 2},
    //{ 13, __NR_rt_sigaction, 4},   // wrapped to use my_ version
    { 14, __NR_rt_sigprocmask, 4},
    { 16, __NR_ioctl, 3},
    { 20, __NR_writev, 3},
    #ifdef __NR_access
    { 21, __NR_access, 2},
    #endif
    #ifdef __NR_pipe
    { 22, __NR_pipe, 1},
    #endif
    #ifdef __NR_select
    { 23, __NR_select, 5},
    #endif
    //{ 25, __NR_mremap, 5},    // wrapped to track protection
    { 39, __NR_getpid, 0},
    { 46, __NR_sendmsg, 3},
    { 47, __NR_recvmsg, 3},
    { 53, __NR_socketpair, 4},
    #ifdef __NR_fork
    { 57, __NR_fork, 0 },    // should wrap this one, because of the struct pt_regs (the only arg)?
    #endif
    { 61, __NR_wait4, 4},
    { 66, __NR_semctl, 4},
    #ifdef __NR_getdents
    { 78, __NR_getdents, 3},
    #endif
    #ifdef __NR_mkdir
    { 83, __NR_mkdir, 2},
    #endif
    #ifdef __NR_unlink
    { 87, __NR_unlink, 1},
    #endif
    //{ 89, __NR_readlink, 3},  // not always existing, better use the wrapped version anyway
    { 96, __NR_gettimeofday, 2},
    { 97, __NR_getrlimit, 2},
    { 101, __NR_ptrace, 4},
    { 125, __NR_capget, 2},
    { 126, __NR_capset, 2},
    { 127, __NR_rt_sigpending, 2},
    //{ 131, __NR_sigaltstack, 2},  // wrapped to use my_sigaltstack
    { 157, __NR_prctl, 5 },     // needs wrapping?
    //{ 158, __NR_arch_prctl, 2},   //need wrapping
    { 186, __NR_gettid, 0 },    //0xBA
    { 200, __NR_tkill, 2 },
    #ifdef __NR_time
    { 201, __NR_time, 1},
    #endif
    { 202, __NR_futex, 6},
    { 217, __NR_getdents64, 3},
    { 220, __NR_semtimedop, 4},
    { 228, __NR_clock_gettime, 2},
    { 229, __NR_clock_getres, 2},
    { 230, __NR_clock_nanosleep, 4},
    { 234, __NR_tgkill, 3},
    #ifdef __NR_inotify_init
    { 253, __NR_inotify_init, 0},   //0xFD
    #endif
    { 254, __NR_inotify_add_watch, 3},
    { 255, __NR_inotify_rm_watch, 2},
    { 274, __NR_get_robust_list, 3},
    { 294, __NR_inotify_init1, 1},
    { 298, __NR_perf_event_open, 5},
    { 318, __NR_getrandom, 3},
};

struct mmap_arg_struct {
    unsigned long addr;
    unsigned long len;
    unsigned long prot;
    unsigned long flags;
    unsigned long fd;
    unsigned long offset;
};

#ifndef __NR_getdents
typedef struct x86_linux_dirent_s {
    unsigned long  d_ino;
    unsigned long  d_off;
    unsigned short d_reclen;
    char           d_name[];
} x86_linux_dirent_t;

typedef struct nat_linux_dirent64_s {
    ino64_t        d_ino;
    off64_t        d_off;
    unsigned short d_reclen;
    unsigned char  d_type;
    char           d_name[];
} nat_linux_dirent64_t;

ssize_t DirentFromDirent64(void* dest, void* source, ssize_t count)
{
    nat_linux_dirent64_t *src = (nat_linux_dirent64_t*)source;
    x86_linux_dirent_t *dst = (x86_linux_dirent_t*)dest;
    ssize_t ret = count;
    while(count>0) {
        dst->d_ino = src->d_ino;
        dst->d_reclen = src->d_reclen+1;
        strcpy(dst->d_name, src->d_name);
        dst->d_off = src->d_off?(src->d_off+1):0;
        *(uint8_t*)((uintptr_t)dst + dst->d_reclen -2) = 0;
        *(uint8_t*)((uintptr_t)dst + dst->d_reclen -1) = src->d_type;

        count -= src->d_reclen;
        ret += 1;
        src = (nat_linux_dirent64_t*)(((uintptr_t)src) + src->d_reclen);
        dst = (x86_linux_dirent_t*)(((uintptr_t)dst) + dst->d_reclen);
    }
    return ret;
}

#endif

//struct x86_pt_regs {
//	long ebx;
//	long ecx;
//	long edx;
//	long esi;
//	long edi;
//	long ebp;
//	long eax;
//	int  xds;
//	int  xes;
//	int  xfs;
//	int  xgs;
//	long orig_eax;
//	long eip;
//	int  xcs;
//	long eflags;
//	long esp;
//	int  xss;
//};

//int clone_fn(void* arg)
//{
//    x64emu_t *emu = (x64emu_t*)arg;
//    R_RAX = 0;
//    DynaRun(emu);
//    int ret = R_EAX;
//    FreeX64Emu(&emu);
//    return ret;
//}

void EXPORT x64Syscall(x64emu_t *emu)
{
    RESET_FLAGS(emu);
    uint32_t s = R_EAX; // EAX? (syscalls only go up to 547 anyways)
    printf_log(LOG_DEBUG, "%p: Calling syscall 0x%02X (%d) %p %p %p %p %p %p", (void*)R_RIP, s, s, (void*)R_RDI, (void*)R_RSI, (void*)R_RDX, (void*)R_R10, (void*)R_R8, (void*)R_R9); 
    // check wrapper first
    int cnt = sizeof(syscallwrap) / sizeof(scwrap_t);
    for (int i=0; i<cnt; i++) {
        if(syscallwrap[i].x64s == s) {
            int sc = syscallwrap[i].nats;
            switch(syscallwrap[i].nbpars) {
                case 0: *(int64_t*)&R_RAX = syscall(sc); break;
                case 1: *(int64_t*)&R_RAX = syscall(sc, R_RDI); break;
                case 2: if(s==33) {printf_dump(LOG_DEBUG, " => sys_access(\"%s\", %ld)\n", (char*)R_RDI, R_RSI);}; *(int64_t*)&R_RAX = syscall(sc, R_RDI, R_RSI); break;
                case 3: *(int64_t*)&R_RAX = syscall(sc, R_RDI, R_RSI, R_RDX); break;
                case 4: *(int64_t*)&R_RAX = syscall(sc, R_RDI, R_RSI, R_RDX, R_R10); break;
                case 5: *(int64_t*)&R_RAX = syscall(sc, R_RDI, R_RSI, R_RDX, R_R10, R_R8); break;
                case 6: *(int64_t*)&R_RAX = syscall(sc, R_RDI, R_RSI, R_RDX, R_R10, R_R8, R_R9); break;
                default:
                   printf_log(LOG_NONE, "ERROR, Unimplemented syscall wrapper (%d, %d)\n", s, syscallwrap[i].nbpars); 
                   emu->quit = 1;
                   return;
            }
            printf_log(LOG_DEBUG, " => 0x%x\n", R_EAX);
            return;
        }
    }
    switch (s) {
        case 0:  // sys_read
            R_EAX = (uint32_t)read((int)R_EDI, (void*)R_RSI, (size_t)R_RDX);
            break;
        case 1:  // sys_write
            R_EAX = (uint32_t)write((int)R_EDI, (void*)R_RSI, (size_t)R_RDX);
            break;
        case 2: // sys_open
            if(s==5) {printf_log(LOG_DEBUG, " => sys_open(\"%s\", %d, %d)", (char*)R_RDI, of_convert(R_ESI), R_EDX);}; 
            //R_EAX = (uint32_t)open((void*)R_EDI, of_convert(R_ESI), R_EDX);
            R_EAX = (uint32_t)my_open(emu, (void*)R_RDI, of_convert(R_ESI), R_EDX);
            break;
        case 3:  // sys_close
            R_EAX = (uint32_t)close((int)R_EDI);
            break;
        case 4: // sys_stat
            R_EAX = (uint32_t)my_stat(emu, (void*)R_RDI, (void*)R_RSI);
            break;
        case 9: // sys_mmap
            R_RAX = (uintptr_t)my_mmap64(emu, (void*)R_RDI, R_RSI, (int)R_EDX, (int)R_R10d, (int)R_R8d, R_R9);
            break;
        case 10: // sys_mprotect
            R_EAX = (uint32_t)my_mprotect(emu, (void*)R_RDI, R_RSI, (int)R_EDX);
            break;
        case 11: // sys_munmap
            R_EAX = (uint32_t)my_munmap(emu, (void*)R_RDI, R_RSI);
            break;
        case 13: // sys_rt_sigaction
            R_EAX = (uint32_t)my_sigaction(emu, (int)R_EDI, (const x64_sigaction_t *)R_RSI, (x64_sigaction_t *)R_RDX/*, (size_t)R_R10*/);
            break;
        #ifndef __NR_access
        case 21: // sys_access
            R_EAX = (uint32_t)access((void*)R_RDI, R_ESI);
            break;
        #endif
        #ifndef __NR_pipe
        case 22:
            R_EAX = (uint32_t)pipe((void*)R_RDI);
            break;
        #endif
        #ifndef __NR_select
        case 23: // sys_select
            R_EAX = (uint32_t)select(R_RDI, (void*)R_RSI, (void*)R_RDX, (void*)R_R10, (void*)R_R8);
            break;
        #endif
        case 25: // sys_mremap
            R_RAX = (uintptr_t)my_mremap(emu, (void*)R_RDI, R_RSI, R_RDX, R_R10d, (void*)R_R8);
            break;
        #ifndef __NR_fork
        case 57: 
            R_RAX = fork();
            break;
        #endif
        #ifndef __NR_getdents
        case 78:
            {
                size_t count = R_RDX;
                nat_linux_dirent64_t *d64 = (nat_linux_dirent64_t*)alloca(count);
                ssize_t ret = syscall(__NR_getdents64, R_EDI, d64, count);
                ret = DirentFromDirent64((void*)R_RSI, d64, ret);
                R_RAX = (uint64_t)ret;
            }
            break;
        #endif
        #ifndef __NR_mkdir
        case 83: // sys_mkdir
            R_EAX = (uint32_t)mkdir((void*)R_RDI, R_ESI);
            break;
        #endif
        #ifndef __NR_unlink
        case 87: //sys_unlink
            R_EAX = (uint32_t)unlink((void*)R_RDI);
            break;
        #endif
        case 89: // sys_readlink
            R_RAX = (ssize_t)my_readlink(emu,(void*)R_RDI, (void*)R_RSI, (size_t)R_RDX);
            break;
        case 131: // sys_sigaltstack
            R_EAX = (uint32_t)my_sigaltstack(emu, (void*)R_RDI, (void*)R_RSI);
            break;
        case 158: // sys_arch_prctl
            R_EAX = (uint32_t)my_arch_prctl(emu, (int)R_EDI, (void*)R_RSI);
            break;
        #ifndef __NR_time
        case 201: // sys_time
            R_RAX = (uintptr_t)time((void*)R_RDI);
            break;
        #endif
        #ifndef __NR_inotify_init
        case 253:
            R_EAX = (int)syscall(__NR_inotify_init1, 0);
            break;
        #endif
        default:
            printf_log(LOG_INFO, "Error: Unsupported Syscall 0x%02Xh (%d)\n", s, s);
            emu->quit = 1;
            emu->error |= ERR_UNIMPL;
            return;
    }
    printf_log(LOG_DEBUG, " => 0x%x\n", R_EAX);
}

#define stack(n) (R_RSP+8+n)
#define i32(n)  *(int32_t*)stack(n)
#define u32(n)  *(uint32_t*)stack(n)
#define i64(n)  *(int64_t*)stack(n)
#define u64(n)  *(uint64_t*)stack(n)
#define p(n)    *(void**)stack(n)

uintptr_t EXPORT my_syscall(x64emu_t *emu)
{
    uint32_t s = R_EDI;;
    printf_dump(LOG_DEBUG, "%p: Calling libc syscall 0x%02X (%d) %p %p %p %p %p\n", (void*)R_RIP, s, s, (void*)R_RSI, (void*)R_RDX, (void*)R_RCX, (void*)R_R8, (void*)R_R9); 
    // check wrapper first
    int cnt = sizeof(syscallwrap) / sizeof(scwrap_t);
    for (int i=0; i<cnt; i++) {
        if(syscallwrap[i].x64s == s) {
            int sc = syscallwrap[i].nats;
            switch(syscallwrap[i].nbpars) {
                case 0: return syscall(sc);
                case 1: return syscall(sc, R_RSI);
                case 2: return syscall(sc, R_RSI, R_RDX);
                case 3: return syscall(sc, R_RSI, R_RDX, R_RCX);
                case 4: return syscall(sc, R_RSI, R_RDX, R_RCX, R_R8);
                case 5: return syscall(sc, R_RSI, R_RDX, R_RCX, R_R8, R_R9);
                case 6: return syscall(sc, R_RSI, R_RDX, R_RCX, R_R8, R_R9, u64(0));
                default:
                   printf_log(LOG_NONE, "ERROR, Unimplemented syscall wrapper (%d, %d)\n", s, syscallwrap[i].nbpars); 
                   emu->quit = 1;
                   return 0;
            }
        }
    }
    switch (s) {
        case 0:  // sys_read
            return (uint32_t)read(R_ESI, (void*)R_RDX, R_ECX);
        case 1:  // sys_write
            return (uint32_t)write(R_ESI, (void*)R_RDX, R_ECX);
        case 2: // sys_open
            return my_open(emu, (char*)R_RSI, of_convert(R_EDX), R_ECX);
        case 3:  // sys_close
            return (uint32_t)close(R_ESI);
        case 4: // sys_stat
            return (uint32_t)my_stat(emu, (void*)R_RSI, (void*)R_RDX);
        case 9: // sys_mmap
            return (uintptr_t)my_mmap64(emu, (void*)R_RSI, R_RDX, (int)R_RCX, (int)R_R8d, (int)R_R9, i64(0));
        case 10: // sys_mprotect
            return (uint32_t)my_mprotect(emu, (void*)R_RSI, R_RDX, (int)R_ECX);
        case 11: // sys_munmap
            return (uint32_t)my_munmap(emu, (void*)R_RSI, R_RDX);
        case 13: // sys_rt_sigaction
            return (uint32_t)my_sigaction(emu, (int)R_ESI, (const x64_sigaction_t *)R_RDX, (x64_sigaction_t *)R_RCX/*, (size_t)R_R8*/);
        #ifndef __NR_access
        case 21: // sys_access
            return (uint32_t)access((void*)R_RSI, R_EDX);
        #endif
        #ifndef __NR_pipe
        case 22:
            return (uint32_t)pipe((void*)R_RSI);
        #endif
        #ifndef __NR_select
        case 23: // sys_select
            return (uint32_t)select(R_RSI, (void*)R_RDX, (void*)R_RCX, (void*)R_R8, (void*)R_R9);
        #endif
        case 25: // sys_mremap
            return (uintptr_t)my_mremap(emu, (void*)R_RSI, R_RDX, R_RCX, R_R8d, (void*)R_R9);
        #ifndef __NR_fork
        case 57: 
            return fork();
        #endif
        #ifndef __NR_getdents
        case 78:
            {
                size_t count = R_RCX;
                nat_linux_dirent64_t *d64 = (nat_linux_dirent64_t*)alloca(count);
                ssize_t ret = syscall(__NR_getdents64, R_ESI, d64, count);
                ret = DirentFromDirent64((void*)R_RDX, d64, ret);
                return (uint64_t)ret;
            }
        #endif
        #ifndef __NR_mkdir
        case 83: // sys_mkdir
            return (uint32_t)mkdir((void*)R_RSI, R_EDX);
        #endif
        #ifndef __NR_unlink
        case 87: //sys_unlink
            return (uint32_t)unlink((void*)R_RSI);
        #endif
        case 89: // sys_readlink
            return (uintptr_t)my_readlink(emu,(void*)R_RSI, (void*)R_RDX, (size_t)R_RCX);
        case 131: // sys_sigaltstack
            return (uint32_t)my_sigaltstack(emu, (void*)R_RSI, (void*)R_RDX);
        case 158: // sys_arch_prctl
            return (uint32_t)my_arch_prctl(emu, (int)R_ESI, (void*)R_RDX);
        #ifndef __NR_time
        case 201: // sys_time
            return (uintptr_t)time((void*)R_RSI);
        #endif
        #ifndef __NR_inotify_init
        case 253:
            return (int)syscall(__NR_inotify_init1, 0);
        #endif
        default:
            printf_log(LOG_INFO, "Error: Unsupported libc Syscall 0x%02X (%d)\n", s, s);
            emu->quit = 1;
            emu->error |= ERR_UNIMPL;
    }
    return 0;
}
