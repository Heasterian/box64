#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/mman.h>

#include "build_info.h"
#include "debug.h"
#include "fileutils.h"
#include "box64context.h"
#include "wine_tools.h"
#include "elfloader.h"
#include "custommem.h"
#include "box64stack.h"
#include "auxval.h"
#include "x64emu.h"
#include "threads.h"
#include "x64trace.h"
#include "librarian.h"
#include "x64run.h"
#include "symbols.h"

box64context_t *my_context = NULL;
int box64_log = LOG_INFO; //LOG_NONE;
int box64_dump = 0;
int box64_nobanner = 0;
int box64_dynarec_log = LOG_NONE;
int box64_pagesize;
#ifdef DYNAREC
int box64_dynarec = 1;
int box64_dynarec_dump = 0;
int box64_dynarec_forced = 0;
uintptr_t box64_nodynarec_start = 0;
uintptr_t box64_nodynarec_end = 0;
#else   //DYNAREC
int box64_dynarec = 0;
#endif
int dlsym_error = 0;
#ifdef HAVE_TRACE
int trace_xmm = 0;
int trace_emm = 0;
int trace_regsdiff = 0;
uint64_t start_cnt = 0;
uintptr_t trace_start = 0, trace_end = 0;
char* trace_func = NULL;
#ifdef DYNAREC
int box64_dynarec_trace = 0;
#endif
#endif
int x11threads = 0;
int x11glx = 1;
int allow_missing_libs = 0;
int fix_64bit_inodes = 0;
int box64_zoom = 0;
int box64_steam = 0;
int box64_wine = 0;
int box64_nopulse = 0;
int box64_nogtk = 0;
int box64_novulkan = 0;
char* libGL = NULL;
uintptr_t fmod_smc_start = 0;
uintptr_t fmod_smc_end = 0;
uint32_t default_gs = 0;
int jit_gdb = 0;
int box64_tcmalloc_minimal = 0;

FILE* ftrace = NULL;
int ftrace_has_pid = 0;

void openFTrace()
{
    char* t = getenv("BOX64_TRACE_FILE");
    char tmp[500];
    char* p = t;
    if(p && strstr(t, "%pid")) {
        strcpy(tmp, p);
        char* c = strstr(tmp, "%pid");
        *c = 0; // cut
        char pid[10];
        sprintf(pid, "%d", getpid());
        strcat(tmp, pid);
        c = strstr(p, "%pid") + strlen("%pid");
        strcat(tmp, c);
        p = tmp;
        ftrace_has_pid = 1;
    }
    if(p) {
        if(!strcmp(p, "stderr"))
            ftrace = stderr;
        else {
            ftrace = fopen(p, "w");
            if(!ftrace) {
                ftrace = stdout;
                printf_log(LOG_INFO, "Cannot open trace file \"%s\" for writing (error=%s)\n", p, strerror(errno));
            } else {
                if(!box64_nobanner)
                    printf("BOX64 Trace redirected to \"%s\"\n", p);
            }
        }
    }
}

void my_child_fork()
{
    if(ftrace_has_pid) {
        // open a new ftrace...
        fclose(ftrace);
        openFTrace();
    }
}


EXPORTDYN
void LoadLogEnv()
{
    ftrace = stdout;
    const char *p = getenv("BOX64_NOBANNER");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_nobanner = p[0]-'0';
        }
    }
    p = getenv("BOX64_LOG");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0'+LOG_NONE && p[0]<='0'+LOG_NEVER) {
                box64_log = p[0]-'0';
                if(box64_log == LOG_NEVER) {
                    --box64_log;
                    box64_dump = 1;
                }
            }
        } else {
            if(!strcasecmp(p, "NONE"))
                box64_log = LOG_NONE;
            else if(!strcasecmp(p, "INFO"))
                box64_log = LOG_INFO;
            else if(!strcasecmp(p, "DEBUG"))
                box64_log = LOG_DEBUG;
            else if(!strcasecmp(p, "DUMP")) {
                box64_log = LOG_DEBUG;
                box64_dump = 1;
            }
        }
        if(!box64_nobanner)
            printf_log(LOG_INFO, "Debug level is %d\n", box64_log);
    }
    p = getenv("BOX64_DUMP");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_dump = p[0]-'0';
        }
    }
    if(!box64_nobanner && box64_dump)
        printf_log(LOG_INFO, "Elf Dump if ON\n");
#ifdef DYNAREC
    p = getenv("BOX64_DYNAREC_DUMP");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='2')
                box64_dynarec_dump = p[0]-'0';
        }
        if (box64_dynarec_dump) printf_log(LOG_INFO, "Dynarec blocks are dumped%s\n", (box64_dynarec_dump>1)?" in color":"");
    }
    p = getenv("BOX64_DYNAREC_LOG");
    if(p) {
        if(strlen(p)==1) {
            if((p[0]>='0'+LOG_NONE) && (p[0]<='0'+LOG_NEVER))
                box64_dynarec_log = p[0]-'0';
        } else {
            if(!strcasecmp(p, "NONE"))
                box64_dynarec_log = LOG_NONE;
            else if(!strcasecmp(p, "INFO"))
                box64_dynarec_log = LOG_INFO;
            else if(!strcasecmp(p, "DEBUG"))
                box64_dynarec_log = LOG_DEBUG;
            else if(!strcasecmp(p, "VERBOSE"))
                box64_dynarec_log = LOG_VERBOSE;
        }
        printf_log(LOG_INFO, "Dynarec log level is %d\n", box64_dynarec_log);
    }
    p = getenv("BOX64_DYNAREC");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_dynarec = p[0]-'0';
        }
        printf_log(LOG_INFO, "Dynarec is %s\n", box64_dynarec?"On":"Off");
    }
    p = getenv("BOX64_DYNAREC_FORCED");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='1')
                box64_dynarec_forced = p[0]-'0';
        }
        if(box64_dynarec_forced)
        printf_log(LOG_INFO, "Dynarec is Forced on all addresses\n");
    }
    p = getenv("BOX64_NODYNAREC");
    if(p) {
        if (strchr(p,'-')) {
            if(sscanf(p, "%ld-%ld", &box64_nodynarec_start, &box64_nodynarec_end)!=2) {
                if(sscanf(p, "0x%lX-0x%lX", &box64_nodynarec_start, &box64_nodynarec_end)!=2)
                    sscanf(p, "%lx-%lx", &box64_nodynarec_start, &box64_nodynarec_end);
            }
            printf_log(LOG_INFO, "No Dynablock creation that start in %p - %p range\n", (void*)box64_nodynarec_start, (void*)box64_nodynarec_end);
        }
    }

#endif
#ifdef HAVE_TRACE
    p = getenv("BOX64_TRACE_XMM");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                trace_xmm = p[0]-'0';
        }
    }
    p = getenv("BOX64_TRACE_EMM");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                trace_emm = p[0]-'0';
        }
    }
    p = getenv("BOX64_TRACE_COLOR");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                trace_regsdiff = p[0]-'0';
        }
    }
    p = getenv("BOX64_TRACE_START");
    if(p) {
        char* p2;
        start_cnt = strtoll(p, &p2, 10);
        printf_log(LOG_INFO, "Will start trace only after %lu instructions\n", start_cnt);
    }
#ifdef DYNAREC
    p = getenv("BOX64_DYNAREC_TRACE");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box64_dynarec_trace = p[0]-'0';
            if(box64_dynarec_trace)
                printf_log(LOG_INFO, "Dynarec generated code will also print a trace\n");
        }
    }
#endif
#endif
    // grab BOX64_TRACE_FILE envvar, and change %pid to actual pid is present in the name
    openFTrace();
    // Other BOX64 env. var.
    p = getenv("BOX64_DLSYM_ERROR");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                dlsym_error = p[0]-'0';
        }
    }
    p = getenv("BOX64_X11THREADS");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                x11threads = p[0]-'0';
        }
        if(x11threads)
            printf_log(LOG_INFO, "Try to Call XInitThreads if libX11 is loaded\n");
    }
    p = getenv("BOX64_X11GLX");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                x11glx = p[0]-'0';
        }
        if(x11glx)
            printf_log(LOG_INFO, "Hack to force libX11 GLX extension present\n");
        else
            printf_log(LOG_INFO, "Disabled Hack to force libX11 GLX extension present\n");
    }
    p = getenv("BOX64_LIBGL");
    if(p)
        libGL = strdup(p);
    if(!libGL) {
        p = getenv("SDL_VIDEO_GL_DRIVER");
        if(p)
            libGL = strdup(p);
    }
    if(libGL) {
        printf_log(LOG_INFO, "BOX64 using \"%s\" as libGL.so.1\n", p);
    }
    p = getenv("BOX64_ALLOWMISSINGLIBS");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                allow_missing_libs = p[0]-'0';
        }
        if(allow_missing_libs)
            printf_log(LOG_INFO, "Allow missing needed libs\n");
    }
    p = getenv("BOX64_NOPULSE");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box64_nopulse = p[0]-'0';
        }
        if(box64_nopulse)
            printf_log(LOG_INFO, "Disable the use of pulseaudio libs\n");
    }
    p = getenv("BOX64_NOGTK");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box64_nogtk = p[0]-'0';
        }
        if(box64_nogtk)
            printf_log(LOG_INFO, "Disable the use of wrapped gtk libs\n");
    }
    p = getenv("BOX64_NOVULKAN");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                box64_novulkan = p[0]-'0';
        }
        if(box64_novulkan)
            printf_log(LOG_INFO, "Disable the use of wrapped vulkan libs\n");
    }
    p = getenv("BOX64_FIX_64BIT_INODES");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                fix_64bit_inodes = p[0]-'0';
        }
        if(fix_64bit_inodes)
            printf_log(LOG_INFO, "Fix 64bit inodes\n");
    }
    p = getenv("BOX64_JITGDB");
        if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[0]<='0'+1)
                jit_gdb = p[0]-'0';
        }
        if(jit_gdb)
            printf_log(LOG_INFO, "Launch %s on segfault\n", (jit_gdb==2)?"gdbserver":"gdb");
    }
    box64_pagesize = sysconf(_SC_PAGESIZE);
    if(!box64_pagesize)
        box64_pagesize = 4096;
}

EXPORTDYN
void LoadEnvPath(path_collection_t *col, const char* defpath, const char* env)
{
    const char* p = getenv(env);
    if(p) {
        printf_log(LOG_INFO, "%s: ", env);
        ParseList(p, col, 1);
    } else {
        printf_log(LOG_INFO, "Using default %s: ", env);
        ParseList(defpath, col, 1);
    }
    if(LOG_INFO<=box64_log) {
        for(int i=0; i<col->size; i++)
            printf_log(LOG_INFO, "%s%s", col->paths[i], (i==col->size-1)?"\n":":");
    }
}

EXPORTDYN
int CountEnv(const char** env)
{
    // count, but remove all BOX64_* environnement
    // also remove PATH and LD_LIBRARY_PATH
    // but add 2 for default BOX64_PATH and BOX64_LD_LIBRARY_PATH
    const char** p = env;
    int c = 0;
    while(*p) {
        if(strncmp(*p, "BOX64_", 6)!=0)
            //if(!(strncmp(*p, "PATH=", 5)==0 || strncmp(*p, "LD_LIBRARY_PATH=", 16)==0))
                ++c;
        ++p;
    }
    return c+2;
}
EXPORTDYN
int GatherEnv(char*** dest, const char** env, const char* prog)
{
    // Add all but BOX64_* environnement
    // but add 2 for default BOX64_PATH and BOX64_LD_LIBRARY_PATH
    const char** p = env;    
    int idx = 0;
    int path = 0;
    int ld_path = 0;
    while(*p) {
        if(strncmp(*p, "BOX64_PATH=", 11)==0) {
            (*dest)[idx++] = strdup(*p+6);
            path = 1;
        } else if(strncmp(*p, "BOX64_LD_LIBRARY_PATH=", 22)==0) {
            (*dest)[idx++] = strdup(*p+6);
            ld_path = 1;
        } else if(strncmp(*p, "_=", 2)==0) {
            /*int l = strlen(prog);
            char tmp[l+3];
            strcpy(tmp, "_=");
            strcat(tmp, prog);
            (*dest)[idx++] = strdup(tmp);*/
        } else if(strncmp(*p, "BOX64_", 6)!=0) {
            (*dest)[idx++] = strdup(*p);
            /*if(!(strncmp(*p, "PATH=", 5)==0 || strncmp(*p, "LD_LIBRARY_PATH=", 16)==0)) {
            }*/
        }
        ++p;
    }
    // update the calloc of envv when adding new variables here
    if(!path) {
        (*dest)[idx++] = strdup("BOX64_PATH=.:bin");
    }
    if(!ld_path) {
        (*dest)[idx++] = strdup("BOX64_LD_LIBRARY_PATH=.:lib");
    }
    // add "_=prog" at the end...
    if(prog) {
        int l = strlen(prog);
        char tmp[l+3];
        strcpy(tmp, "_=");
        strcat(tmp, prog);
        (*dest)[idx++] = strdup(tmp);
    }
    // and a final NULL
    (*dest)[idx++] = 0;
    return 0;
}


void PrintHelp() {
    printf("\n\nThis is Box64, the Linux x86_64 emulator with a twist\n");
    printf("\nUsage is box64 [options] path/to/software [args]\n");
    printf("to launch x86_64 software\n");
    printf(" options can be :\n");
    printf("    '-v'|'--version' to print box64 version and quit\n");
    printf("    '-h'|'--help'    to print box64 help and quit\n");
    printf("You can also set some environment variables:\n");
    printf(" BOX64_PATH is the box64 version of PATH (default is '.:bin')\n");
    printf(" BOX64_LD_LIBRARY_PATH is the box64 version LD_LIBRARY_PATH (default is '.:lib')\n");
    printf(" BOX64_LOG with 0/1/2/3 or NONE/INFO/DEBUG/DUMP to set the printed debug info (level 3 is level 2 + BOX64_DUMP)\n");
    printf(" BOX64_DUMP with 0/1 to dump elf infos\n");
    printf(" BOX64_NOBANNER with 0/1 to enable/disable the printing of box64 version and build at start\n");
#ifdef DYNAREC
    printf(" BOX64_DYNAREC_LOG with 0/1/2/3 or NONE/INFO/DEBUG/DUMP to set the printed dynarec info\n");
    printf(" BOX64_DYNAREC with 0/1 to disable or enable Dynarec (On by default)\n");
    printf(" BOX64_NODYNAREC with address interval (0x1234-0x4567) to forbid dynablock creation in the interval specified\n");
#endif
#ifdef HAVE_TRACE
    printf(" BOX64_TRACE with 1 to enable x86_64 execution trace\n");
    printf("    or with XXXXXX-YYYYYY to enable x86_64 execution trace only between address\n");
    printf("    or with FunctionName to enable x86_64 execution trace only in one specific function\n");
    printf("  use BOX64_TRACE_INIT instead of BOX_TRACE to start trace before init of Libs and main program\n\t (function name will probably not work then)\n");
    printf(" BOX64_TRACE_EMM with 1 to enable dump of MMX registers along with regular registers\n");
    printf(" BOX64_TRACE_XMM with 1 to enable dump of SSE registers along with regular registers\n");
    printf(" BOX64_TRACE_COLOR with 1 to enable detection of changed general register values\n");
    printf(" BOX64_TRACE_START with N to enable trace after N instructions\n");
#ifdef DYNAREC
    printf(" BOX64_DYNAREC_TRACE with 0/1 to disable or enable Trace on generated code too\n");
#endif
#endif
    printf(" BOX64_TRACE_FILE with FileName to redirect logs in a file (or stderr to use stderr instead of stdout)");
    printf(" BOX64_DLSYM_ERROR with 1 to log dlsym errors\n");
    printf(" BOX64_LOAD_ADDR=0xXXXXXX try to load at 0xXXXXXX main binary (if binary is a PIE)\n");
    printf(" BOX64_NOSIGSEGV=1 to disable handling of SigSEGV\n");
    printf(" BOX64_NOSIGILL=1  to disable handling of SigILL\n");
    printf(" BOX64_X11THREADS=1 to call XInitThreads when loading X11 (for old Loki games with Loki_Compat lib)");
    printf(" BOX64_LIBGL=libXXXX set the name (and optionnaly full path) for libGL.so.1\n");
    printf(" BOX64_LD_PRELOAD=XXXX[:YYYYY] force loading XXXX (and YYYY...) libraries with the binary\n");
    printf(" BOX64_ALLOWMISSINGLIBS with 1 to allow to continue even if a lib is missing (unadvised, will probably  crash later)\n");
    printf(" BOX64_NOPULSE=1 to disable the loading of pulseaudio libs\n");
    printf(" BOX64_NOGTK=1 to disable the loading of wrapped gtk libs\n");
    printf(" BOX64_NOVULKAN=1 to disable the loading of wrapped vulkan libs\n");
    printf(" BOX64_JITGDB with 1 to launch \"gdb\" when a segfault is trapped, attached to the offending process\n");
}

EXPORTDYN
void LoadEnvVars(box64context_t *context)
{
    // check BOX64_LD_LIBRARY_PATH and load it
    LoadEnvPath(&context->box64_ld_lib, ".:lib:lib64:x86_64:bin64:libs64", "BOX64_LD_LIBRARY_PATH");
    if(FileExist("/lib/x86_64-linux-gnu", 0))
        AddPath("/lib/x86_64-linux-gnu", &context->box64_ld_lib, 1);
    if(FileExist("/usr/lib/x86_64-linux-gnu", 0))
        AddPath("/usr/lib/x86_64-linux-gnu", &context->box64_ld_lib, 1);
    if(getenv("LD_LIBRARY_PATH"))
        PrependList(&context->box64_ld_lib, getenv("LD_LIBRARY_PATH"), 1);   // in case some of the path are for x86 world
    if(getenv("BOX64_EMULATED_LIBS")) {
        char* p = getenv("BOX64_EMULATED_LIBS");
        ParseList(p, &context->box64_emulated_libs, 0);
        if (my_context->box64_emulated_libs.size && box64_log) {
            printf_log(LOG_INFO, "BOX64 will force the used of emulated libs for ");
            for (int i=0; i<context->box64_emulated_libs.size; ++i)
                printf_log(LOG_INFO, "%s ", context->box64_emulated_libs.paths[i]);
            printf_log(LOG_INFO, "\n");
        }
    }

    if(getenv("BOX64_NOSIGSEGV")) {
        if (strcmp(getenv("BOX64_NOSIGSEGV"), "1")==0)
            context->no_sigsegv = 1;
            printf_log(LOG_INFO, "BOX64: Disabling handling of SigSEGV\n");
    }
    if(getenv("BOX64_NOSIGILL")) {
        if (strcmp(getenv("BOX64_NOSIGILL"), "1")==0)
            context->no_sigill = 1;
            printf_log(LOG_INFO, "BOX64: Disabling handling of SigILL\n");
    }
    // check BOX64_PATH and load it
    LoadEnvPath(&context->box64_path, ".:bin", "BOX64_PATH");
    if(getenv("PATH"))
        AppendList(&context->box64_path, getenv("PATH"), 1);   // in case some of the path are for x86 world
#ifdef HAVE_TRACE
    char* p = getenv("BOX64_TRACE");
    if(p) {
        if (strcmp(p, "0"))
            context->x64trace = 1;
    }
    p = getenv("BOX64_TRACE_INIT");
    if(p) {
        if (strcmp(p, "0"))
            context->x64trace = 1;
    }
    if(my_context->x64trace) {
        printf_log(LOG_INFO, "Initializing Zydis lib\n");
        if(InitX64Trace(my_context)) {
            printf_log(LOG_INFO, "Zydis init failed, no x86 trace activated\n");
            context->x64trace = 0;
        }
    }
#endif
}

EXPORTDYN
void setupTraceInit()
{
#ifdef HAVE_TRACE
    char* p = getenv("BOX64_TRACE_INIT");
    if(p) {
        setbuf(stdout, NULL);
        uintptr_t s_trace_start=0, s_trace_end=0;
        if (strcmp(p, "1")==0)
            SetTraceEmu(0, 0);
        else if (strchr(p,'-')) {
            if(sscanf(p, "%ld-%ld", &s_trace_start, &s_trace_end)!=2) {
                if(sscanf(p, "0x%lX-0x%lX", &s_trace_start, &s_trace_end)!=2)
                    sscanf(p, "%lx-%lx", &s_trace_start, &s_trace_end);
            }
            if(s_trace_start || s_trace_end)
                SetTraceEmu(s_trace_start, s_trace_end);
        } else {
            if (GetSymbolStartEnd(GetMapSymbol(my_context->maplib), p, &s_trace_start, &s_trace_end, -1, NULL, -1)) {
                SetTraceEmu(s_trace_start, s_trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
            } else {
                printf_log(LOG_NONE, "Warning, symbol to trace (\"%s\") not found, disabling trace\n", p);
                SetTraceEmu(0, 100);  // disabling trace, mostly
            }
        }
    } else {
        p = getenv("BOX64_TRACE");
        if(p)
            if (strcmp(p, "0"))
                SetTraceEmu(0, 1);
    }
#endif
}

EXPORTDYN
void setupTrace()
{
#ifdef HAVE_TRACE
    char* p = getenv("BOX64_TRACE");
    if(p) {
        setbuf(stdout, NULL);
        uintptr_t s_trace_start=0, s_trace_end=0;
        if (strcmp(p, "1")==0)
            SetTraceEmu(0, 0);
        else if (strchr(p,'-')) {
            if(sscanf(p, "%ld-%ld", &s_trace_start, &s_trace_end)!=2) {
                if(sscanf(p, "0x%lX-0x%lX", &s_trace_start, &s_trace_end)!=2)
                    sscanf(p, "%lx-%lx", &s_trace_start, &s_trace_end);
            }
            if(s_trace_start || s_trace_end) {
                SetTraceEmu(s_trace_start, s_trace_end);
                if(!s_trace_start && s_trace_end==1) {
                    printf_log(LOG_INFO, "TRACE enabled but inactive\n");
                } else {
                    printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
                }
            }
        } else {
            if (GetGlobalSymbolStartEnd(my_context->maplib, p, &s_trace_start, &s_trace_end, NULL, -1, NULL)) {
                SetTraceEmu(s_trace_start, s_trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
            } else if(GetLocalSymbolStartEnd(my_context->maplib, p, &s_trace_start, &s_trace_end, NULL, -1, NULL)) {
                SetTraceEmu(s_trace_start, s_trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)s_trace_start, (void*)s_trace_end);
            } else {
                printf_log(LOG_NONE, "Warning, symbol to trace (\"%s\") not found, trying to set trace later\n", p);
                SetTraceEmu(0, 1);  // disabling trace, mostly
                trace_func = strdup(p);
            }
        }
    }
#endif
}

void endBox64()
{
    if(!my_context)
        return;

    x64emu_t* emu = thread_get_emu();
    // atexit first
    printf_log(LOG_DEBUG, "Calling atexit registered functions (exiting box64)\n");
    CallAllCleanup(emu);
    // then call all the fini
    printf_log(LOG_DEBUG, "Calling fini for all loaded elfs and unload native libs\n");
    RunElfFini(my_context->elfs[0], emu);
    #ifdef DYNAREC
    // disable dynarec now
    box64_dynarec = 0;
    #endif
    FreeLibrarian(&my_context->local_maplib, emu);    // unload all libs
    FreeLibrarian(&my_context->maplib, emu);    // unload all libs
    // waiting for all thread except this one to finish
    int this_thread = GetTID();
    int pid = getpid();
    int running = 1;
    int attempt = 0;
    printf_log(LOG_DEBUG, "Waiting for all threads to finish before unloading box64context\n");
    while(running) {
        DIR *proc_dir;
        char dirname[100];
        snprintf(dirname, sizeof dirname, "/proc/self/task");
        proc_dir = opendir(dirname);
        running = 0;
        if (proc_dir)
        {
            struct dirent *entry;
            while ((entry = readdir(proc_dir)) != NULL && !running)
            {
                if(entry->d_name[0] == '.')
                    continue;

                int tid = atoi(entry->d_name);
                // tid != pthread_t, so no pthread functions are available here
                if(tid!=this_thread) {
                    if(attempt>4000) {
                        printf_log(LOG_INFO, "Stop waiting for remaining thread %04d\n", tid);
                        // enough wait, kill all thread!
                        syscall(__NR_tgkill, pid, tid, SIGABRT);
                    } else {
                        running = 1;
                        ++attempt;
                        sched_yield();
                    }
                }
            }
            closedir(proc_dir);
        }
    }
    // all done, free context
    FreeBox64Context(&my_context);
    if(libGL) {
        free(libGL);
        libGL = NULL;
    }
}


static void free_contextargv()
{
    for(int i=0; i<my_context->argc; ++i)
        free(my_context->argv[i]);
}

const char **environ __attribute__((weak)) = NULL;
int main(int argc, const char **argv, const char **env) {

    init_auxval(argc, argv, environ?environ:env);
    // trying to open and load 1st arg
    if(argc==1) {
        PrintBox64Version();
        PrintHelp();
        return 1;
    }
    if(argc>1 && !strcmp(argv[1], "/usr/bin/gdb") && getenv("BOX64_TRACE_FILE"))
        exit(0);

    // init random seed
    srandom(time(NULL));

    // check BOX64_LOG debug level
    LoadLogEnv();
    
    const char* prog = argv[1];
    int nextarg = 1;
    // check if some options are passed
    while(prog && prog[0]=='-') {
        if(!strcmp(prog, "-v") || !strcmp(prog, "--version")) {
            PrintBox64Version();
            exit(0);
        }
        if(!strcmp(prog, "-h") || !strcmp(prog, "--help")) {
            PrintHelp();
            exit(0);
        }
        // other options?
        if(!strcmp(prog, "--")) {
            prog = argv[++nextarg];
            break;
        }
        printf("Warning, unrecognized option '%s'\n", prog);
        prog = argv[++nextarg];
    }
    if(!prog || nextarg==argc) {
        printf("Box64: nothing to run\n");
        exit(0);
    }
    if(!box64_nobanner)
        PrintBox64Version();
    // precheck, for win-preload
    if(strstr(prog, "wine-preloader")==(prog+strlen(prog)-strlen("wine-preloader")) 
     || strstr(prog, "wine64-preloader")==(prog+strlen(prog)-strlen("wine64-preloader"))) {
        // wine-preloader detecter, skipping it if next arg exist and is an x86 binary
        int x64 = (nextarg<argc)?FileIsX64ELF(argv[nextarg]):0;
        if(x64) {
            prog = argv[++nextarg];
            printf_log(LOG_INFO, "BOX64: Wine preloader detected, loading \"%s\" directly\n", prog);
            //wine_preloaded = 1;
        }
    }
    // check if this is wine
    if(!strcmp(prog, "wine64")
     || !strcmp(prog, "wine64-development") 
     || (strlen(prog)>5 && !strcmp(prog+strlen(prog)-strlen("/wine64"), "/wine64"))) {
        const char* prereserve = getenv("WINEPRELOADRESERVE");
        printf_log(LOG_INFO, "BOX64: Wine64 detected, WINEPRELOADRESERVE=\"%s\"\n", prereserve?prereserve:"");
        if(wine_preloaded)
            wine_prereserve(prereserve);
        // special case for winedbg, doesn't work anyway
        if(argv[nextarg+1] && strstr(argv[nextarg+1], "winedbg")==argv[nextarg+1]) {
            printf_log(LOG_NONE, "winedbg detected, not launching it!\n");
            exit(0);    // exiting, it doesn't work anyway
        }
        box64_wine = 1;
    }
    // check if this is wineserver
    if(!strcmp(prog, "wineserver") || !strcmp(prog, "wineserver64") || (strlen(prog)>9 && !strcmp(prog+strlen(prog)-strlen("/wineserver"), "/wineserver"))) {
        box64_wine = 1;
    }
    // Create a new context
    my_context = NewBox64Context(argc - nextarg);

    // check BOX64_LD_LIBRARY_PATH and load it
    LoadEnvVars(my_context);

    if(argv[0][0]=='/')
        my_context->box64path = strdup(argv[0]);
    else
        my_context->box64path = ResolveFile(argv[0], &my_context->box64_path);
    // prepare all other env. var
    my_context->envc = CountEnv(environ?environ:env);
    printf_log(LOG_INFO, "Counted %d Env var\n", my_context->envc);
    // allocate extra space for new environment variables such as BOX64_PATH
    my_context->envv = (char**)calloc(my_context->envc+4, sizeof(char*));
    GatherEnv(&my_context->envv, environ?environ:env, my_context->box64path);
    if(box64_dump) {
        for (int i=0; i<my_context->envc; ++i)
            printf_dump(LOG_NEVER, " Env[%02d]: %s\n", i, my_context->envv[i]);
    }

    path_collection_t ld_preload = {0};
    if(getenv("BOX64_LD_PRELOAD")) {
        char* p = getenv("BOX64_LD_PRELOAD");
        ParseList(p, &ld_preload, 0);
        if (ld_preload.size && box64_log) {
            printf_log(LOG_INFO, "BOX64 try to Preload ");
            for (int i=0; i<ld_preload.size; ++i)
                printf_log(LOG_INFO, "%s ", ld_preload.paths[i]);
            printf_log(LOG_INFO, "\n");
        }
    } else {
        if(getenv("LD_PRELOAD")) {
            char* p = getenv("LD_PRELOAD");
            if(strstr(p, "libtcmalloc_minimal.so.4"))
                box64_tcmalloc_minimal = 1;
            if(strstr(p, "libtcmalloc_minimal_debug.so.4"))
                box64_tcmalloc_minimal = 1;
            if(strstr(p, "libasan.so"))
                box64_tcmalloc_minimal = 1; // it seems Address Sanitizer doesn't handle dlsym'd malloc very well
            ParseList(p, &ld_preload, 0);
            if (ld_preload.size && box64_log) {
                printf_log(LOG_INFO, "BOX64 try to Preload ");
                for (int i=0; i<ld_preload.size; ++i)
                    printf_log(LOG_INFO, "%s ", ld_preload.paths[i]);
                printf_log(LOG_INFO, "\n");
            }
        }
    }
    // lets build argc/argv stuff
    printf_log(LOG_INFO, "Looking for %s\n", prog);
    if(strchr(prog, '/'))
        my_context->argv[0] = strdup(prog);
    else
        my_context->argv[0] = ResolveFile(prog, &my_context->box64_path);

    const char* prgname = strrchr(prog, '/');
    if(!prgname)
        prgname = prog;
    else
        ++prgname;
    // special case for LittleInferno that use an old libvorbis
    if(strstr(prgname, "LittleInferno.bin.x86")==prgname) {
        printf_log(LOG_INFO, "LittleInferno detected, forcing emulated libvorbis\n");
        AddPath("libvorbis.so.0", &my_context->box64_emulated_libs, 0);
    }
    // special case for dontstarve that use an old SDL2
    if(strstr(prgname, "dontstarve")) {
        printf_log(LOG_INFO, "Dontstarve* detected, forcing emulated SDL2\n");
        AddPath("libSDL2-2.0.so.0", &my_context->box64_emulated_libs, 0);
    }
    // special case for steam that somehow seems to alter libudev opaque pointer (udev_monitor)
    if(!strcmp(prgname, "steam")) {
        printf_log(LOG_INFO, "steam detected, forcing emulated libudev\n");
        AddPath("libudev.so.0", &my_context->box64_emulated_libs, 0);
        box64_steam = 1;
    }
    // special case for steam-runtime-check-requirements to fake 64bits suport
    if(strstr(prgname, "steam-runtime-check-requirements")==prgname) {
        printf_log(LOG_INFO, "steam-runtime-check-requirements detected, faking All is good!\n");
        exit(0);    // exiting, not testing anything
    }
    // special case for UnrealLinux.bin, it doesn't like "full path resolution"
    if(!strcmp(prog, "UnrealLinux.bin") && my_context->argv[0]) {
        free(my_context->argv[0]);
        my_context->argv[0] = strdup("./UnrealLinux.bin");
    }
    // special case for zoom
    if(strstr(prgname, "zoom")==prgname) {
        printf_log(LOG_INFO, "Zoom detected, trying to use system libturbojpeg if possible\n");
        box64_zoom = 1;
    }
    /*if(strstr(prgname, "awesomium_process")==prgname) {
        printf_log(LOG_INFO, "awesomium_process detected, forcing emulated libpng12\n");
        AddPath("libpng12.so.0", &my_context->box64_emulated_libs, 0);
    }*/
    /*if(!strcmp(prgname, "gdb")) {
        exit(-1);
    }*/

    for(int i=1; i<my_context->argc; ++i) {
        my_context->argv[i] = strdup(argv[i+nextarg]);
        printf_log(LOG_INFO, "argv[%i]=\"%s\"\n", i, my_context->argv[i]);
    }

    // check if file exist
    if(!my_context->argv[0] || !FileExist(my_context->argv[0], IS_FILE)) {
        printf_log(LOG_NONE, "Error: file is not found (check BOX64_PATH)\n");
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    if(!FileExist(my_context->argv[0], IS_FILE|IS_EXECUTABLE)) {
        printf_log(LOG_NONE, "Error: %s is not an executable file\n", my_context->argv[0]);
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    if(!(my_context->fullpath = realpath(my_context->argv[0], NULL)))
        my_context->fullpath = strdup(my_context->argv[0]);
    FILE *f = fopen(my_context->argv[0], "rb");
    if(!f) {
        printf_log(LOG_NONE, "Error: Cannot open %s\n", my_context->argv[0]);
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    elfheader_t *elf_header = LoadAndCheckElfHeader(f, my_context->argv[0], 1);
    if(!elf_header) {
        printf_log(LOG_NONE, "Error: reading elf header of %s, try to launch natively instead\n", my_context->argv[0]);
        fclose(f);
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return execvp(argv[1], (char * const*)(argv+1));
    }
    AddElfHeader(my_context, elf_header);

    if(CalcLoadAddr(elf_header)) {
        printf_log(LOG_NONE, "Error: reading elf header of %s\n", my_context->argv[0]);
        fclose(f);
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // allocate memory
    if(AllocElfMemory(my_context, elf_header, 1)) {
        printf_log(LOG_NONE, "Error: allocating memory for elf %s\n", my_context->argv[0]);
        fclose(f);
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // Load elf into memory
    if(LoadElfMemory(f, my_context, elf_header)) {
        printf_log(LOG_NONE, "Error: loading in memory elf %s\n", my_context->argv[0]);
        fclose(f);
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // can close the file now
    fclose(f);
    if(ElfCheckIfUseTCMallocMinimal(elf_header)) {
        if(!box64_tcmalloc_minimal) {
            // need to reload with tcmalloc_minimal as a LD_PRELOAD!
            printf_log(LOG_INFO, "BOX64: tcmalloc_minimal.so.4 used, reloading box64 with the lib preladed\n");
            // need to get a new envv variable. so first count it and check if LD_PRELOAD is there
            int preload=(getenv("LD_PRELOAD"))?1:0;
            int nenv = 0;
            while(env[nenv]) nenv++;
            // alloc + "LD_PRELOAD" if needd + last NULL ending
            char** newenv = (char**)calloc(nenv+1+((preload)?0:1), sizeof(char*));
            // copy strings
            for (int i=0; i<nenv; ++i)
                newenv[i] = strdup(env[i]);
            // add ld_preload
            if(preload) {
                // find the line
                int l = 0;
                while(l<nenv) {
                    if(strstr(newenv[l], "LD_PRELOAD=")==newenv[l]) {
                        // found it!
                        char *old = newenv[l];
                        newenv[l] = (char*)calloc(strlen(old)+strlen("libtcmalloc_minimal.so.4:")+1, sizeof(char));
                        strcpy(newenv[l], "LD_PRELOAD=libtcmalloc_minimal.so.4:");
                        strcat(newenv[l], old + strlen("LD_PRELOAD="));
                        free(old);
                        // done, end loop
                        l = nenv;
                    } else ++l;
                }
            } else {
                //move last one
                newenv[nenv] = strdup(newenv[nenv-1]);
                free(newenv[nenv-1]);
                newenv[nenv-1] = strdup("LD_PRELOAD=libtcmalloc_minimal.so.4");
            }
            // duplicate argv too
            char** newargv = calloc(argc+1, sizeof(char*));
            int narg = 0;
            while(argv[narg]) {newargv[narg] = strdup(argv[narg]); narg++;}
            // launch with new env...
            if(execve(newargv[0], newargv, newenv)<0)
                printf_log(LOG_NONE, "Failed to relaunch, error is %d/%s\n", errno, strerror(errno));
        } else {
            printf_log(LOG_INFO, "BOX64: Using tcmalloc_minimal.so.4, and it's in the LD_PRELOAD command\n");
        }
    }
    // get and alloc stack size and align
    if(CalcStackSize(my_context)) {
        printf_log(LOG_NONE, "Error: allocating stack\n");
        free_contextargv();
        FreeBox64Context(&my_context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // init x86_64 emu
    x64emu_t *emu = NewX64Emu(my_context, my_context->ep, (uintptr_t)my_context->stack, my_context->stacksz, 0);
    // stack setup is much more complicated then just that!
    SetupInitialStack(emu); // starting here, the argv[] don't need free anymore
    SetupX64Emu(emu);
    SetRSI(emu, my_context->argc);
    SetRDX(emu, (uint64_t)my_context->argv);
    SetRCX(emu, (uint64_t)my_context->envv);
    SetRBP(emu, 0); // Frame pointer so to "No more frame pointer"

    // child fork to handle traces
    pthread_atfork(NULL, NULL, my_child_fork);

    thread_set_emu(emu);

    setupTraceInit();
    // export symbols
    AddSymbols(my_context->maplib, GetMapSymbol(my_context->maplib), GetWeakSymbol(my_context->maplib), GetLocalSymbol(my_context->maplib), elf_header);
    if(wine_preloaded) {
        uintptr_t wineinfo = FindSymbol(GetMapSymbol(my_context->maplib), "wine_main_preload_info", -1, NULL, 1);
        if(!wineinfo) wineinfo = FindSymbol(GetWeakSymbol(my_context->maplib), "wine_main_preload_info", -1, NULL, 1);
        if(!wineinfo) wineinfo = FindSymbol(GetLocalSymbol(my_context->maplib), "wine_main_preload_info", -1, NULL, 1);
        if(!wineinfo) {printf_log(LOG_NONE, "Warning, Symbol wine_main_preload_info not found\n");}
        else {
            *(void**)wineinfo = get_wine_prereserve();
            printf_log(LOG_DEBUG, "WINE wine_main_preload_info found and updated\n");
        }
        #ifdef DYNAREC
        dynarec_wine_prereserve();
        #endif
    }
    // pre-load lib if needed
    if(ld_preload.size) {
        if(AddNeededLib(NULL, NULL, NULL, 0, (const char**)ld_preload.paths, ld_preload.size, my_context, emu)) {
            printf_log(LOG_INFO, "Warning, cannot pre-load a lib\n");
        }
    }
    FreeCollection(&ld_preload);
    // Call librarian to load all dependant elf
    if(LoadNeededLibs(elf_header, my_context->maplib, &my_context->neededlibs, NULL, 0, my_context, emu)) {
        printf_log(LOG_NONE, "Error: loading needed libs in elf %s\n", my_context->argv[0]);
        FreeBox64Context(&my_context);
        return -1;
    }
    // reloc...
    printf_log(LOG_DEBUG, "And now export symbols / relocation for %s...\n", ElfName(elf_header));
    if(RelocateElf(my_context->maplib, NULL, elf_header)) {
        printf_log(LOG_NONE, "Error: relocating symbols in elf %s\n", my_context->argv[0]);
        FreeBox64Context(&my_context);
        return -1;
    }
    // and handle PLT
    RelocateElfPlt(my_context->maplib, NULL, elf_header);
    // defered init
    RunDeferedElfInit(emu);
    // do some special case check, _IO_2_1_stderr_ and friends, that are setup by libc, but it's already done here, so need to do a copy
    ResetSpecialCaseMainElf(elf_header);
    // init...
    setupTrace();
    // get entrypoint
    my_context->ep = GetEntryPoint(my_context->maplib, elf_header);
#if defined(RPI) || defined(RK3399) || defined(RK3326)
    // before launching emulation, let's check if this is a mojosetup from GOG
    if (((strstr(prog, "bin/linux/x86/mojosetup") && getenv("MOJOSETUP_BASE")) || strstr(prog, ".mojosetup/mojosetup"))
       && getenv("GTK2_RC_FILES")) {
        sanitize_mojosetup_gtk_background();
    }
#endif

    atexit(endBox64);
    loadProtectionFromMap();

    // emulate!
    printf_log(LOG_DEBUG, "Start x64emu on Main\n");
    // Stack is ready, with stacked: NULL env NULL argv argc
    SetRIP(emu, my_context->ep);
    ResetFlags(emu);
    PushExit(emu);  // push to pop it just after
    SetRDX(emu, Pop64(emu));    // RDX is exit function
    Run(emu, 0);
    // Get EAX
    int ret = GetEAX(emu);
    printf_log(LOG_DEBUG, "Emulation finished, EAX=%d\n", ret);

#ifdef HAVE_TRACE
    if(trace_func)  {
        free(trace_func);
        trace_func = NULL;
    }
#endif

    return ret;
}
