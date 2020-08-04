/* Copyright (c) 2001, 2016, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "my_stacktrace.h"

#ifndef _WIN32
#include "my_thread.h"
#include "m_string.h"
#include <signal.h>
#if HAVE_LIBCOREDUMPER
#include <coredumper/coredumper.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STACKTRACE

#ifdef __linux__
#include <ctype.h>          /* isprint */
#include <sys/syscall.h>    /* SYS_gettid */
#endif

#if HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#ifdef __linux__
/* __bss_start doesn't seem to work on FreeBSD and doesn't exist on OSX/Solaris. */
#define PTR_SANE(p) ((p) && (char*)(p) >= heap_start && (char*)(p) <= heap_end)
static char *heap_start;
extern char *__bss_start;
#else
#define PTR_SANE(p) (p)
#endif /* __linux */

//my_print_buildID
#ifdef __linux__
#include "my_sys.h"
#include <sys/stat.h>
#include <elf.h>
#if defined(__x86_64__)
#define ElfW(type) Elf64_##type
#else
#define ElfW(type) Elf32_##type
#endif
#endif  //defined(OS_LINUX)

void my_init_stacktrace()
{
#ifdef __linux__
  heap_start = (char*) &__bss_start;
#endif /* __linux__ */
}

#ifdef __linux__

static void print_buffer(char *buffer, size_t count)
{
  const char s[]= " ";
  for (; count && *buffer; --count)
  {
    my_write_stderr(isprint(*buffer) ? buffer : s, 1);
    ++buffer;
  }
}

/**
  Access the pages of this process through /proc/self/task/<tid>/mem
  in order to safely print the contents of a memory address range.

  @param  addr      The address at the start of the memory region.
  @param  max_len   The length of the memory region.

  @return Zero on success.
*/
static int safe_print_str(const char *addr, int max_len)
{
  int fd;
  pid_t tid;
  off_t offset;
  ssize_t nbytes= 0;
  size_t total, count;
  char buf[256];

  tid= (pid_t) syscall(SYS_gettid);

  sprintf(buf, "/proc/self/task/%d/mem", tid);

  if ((fd= open(buf, O_RDONLY)) < 0)
    return -1;

  /* Ensure that off_t can hold a pointer. */
  compile_time_assert(sizeof(off_t) >= sizeof(intptr));

  total= max_len;
  offset= (intptr) addr;

  /* Read up to the maximum number of bytes. */
  while (total)
  {
    count= MY_MIN(sizeof(buf), total);

    if ((nbytes= pread(fd, buf, count, offset)) < 0)
    {
      /* Just in case... */
      if (errno == EINTR)
        continue;
      else
        break;
    }

    /* Advance offset into memory. */
    total-= nbytes;
    offset+= nbytes;
    addr+= nbytes;

    /* Output the printable characters. */
    print_buffer(buf, nbytes);

    /* Break if less than requested... */
    if ((count - nbytes))
      break;
  }

  /* Output a new line if something was printed. */
  if (total != (size_t) max_len)
    my_safe_printf_stderr("%s", "\n");

  if (nbytes == -1)
    my_safe_printf_stderr("Can't read from address %p\n", addr);

  close(fd);

  return 0;
}

#endif /* __linux __ */

void my_safe_puts_stderr(const char* val, size_t max_len)
{
#ifdef __linux__
/* Only needed by the linux version of PTR_SANE */
  char *heap_end;

  if (!safe_print_str(val, max_len))
    return;

  heap_end= (char*) sbrk(0);
#endif

  if (!PTR_SANE(val))
  {
    my_safe_printf_stderr("%s", "is an invalid pointer\n");
    return;
  }

  for (; max_len && PTR_SANE(val) && *val; --max_len)
    my_write_stderr((val++), 1);
  my_safe_printf_stderr("%s", "\n");
}

#if defined(HAVE_PRINTSTACK)

/* Use Solaris' symbolic stack trace routine. */
#include <ucontext.h>

void my_print_stacktrace(uchar* stack_bottom MY_ATTRIBUTE((unused)), 
                         ulong thread_stack MY_ATTRIBUTE((unused)))
{
  if (printstack(fileno(stderr)) == -1)
    my_safe_printf_stderr("%s",
      "Error when traversing the stack, stack appears corrupt.\n");
  else
    my_safe_printf_stderr("Please read "
      "http://dev.mysql.com/doc/refman/%u.%u/en/resolve-stack-dump.html\n"
      "and follow instructions on how to resolve the stack trace.\n"
      "Resolved stack trace is much more helpful in diagnosing the\n"
      "problem, so please do resolve it\n",
      MYSQL_VERSION_MAJOR, MYSQL_VERSION_MINOR);
}

#elif HAVE_BACKTRACE

#if HAVE_ABI_CXA_DEMANGLE

char MY_ATTRIBUTE ((weak)) *
my_demangle(const char *mangled_name MY_ATTRIBUTE((unused)),
            int *status MY_ATTRIBUTE((unused)))
{
  return NULL;
}

static void my_demangle_symbols(char **addrs, int n)
{
  int status, i;
  char *begin, *end, *demangled;

  for (i= 0; i < n; i++)
  {
    demangled= NULL;
    begin= strchr(addrs[i], '(');
    end= begin ? strchr(begin, '+') : NULL;

    if (begin && end)
    {
      *begin++= *end++= '\0';
      demangled= my_demangle(begin, &status);
      if (!demangled || status)
      {
        demangled= NULL;
        begin[-1]= '(';
        end[-1]= '+';
      }
    }

    if (demangled)
      my_safe_printf_stderr("%s(%s+%s\n", addrs[i], demangled, end);
    else
      my_safe_printf_stderr("%s\n", addrs[i]);
  }
}

#endif /* HAVE_ABI_CXA_DEMANGLE */
#ifdef __linux__
void my_print_buildID()
{
  int         readlink_bytes;
  char        proc_path[FN_REFLEN];
  char        mysqld_path[PATH_MAX + 1]= {0};
  FILE *      mysqld;
  struct stat statbuf;
  ElfW(Ehdr) *ehdr= 0;
  ElfW(Phdr) *phdr= 0;
  ElfW(Nhdr) *nhdr= 0;
  unsigned char *build_id;
  char           buff[3];
  uint           i;
  snprintf(proc_path, sizeof(proc_path), "/proc/%d/exe", getpid());
  readlink_bytes= readlink(proc_path, mysqld_path, sizeof(mysqld_path));
  if (readlink_bytes < 1)
  {
    my_safe_printf_stderr("Error reading process path\n");
    return;
  }
  if (!(mysqld= fopen(mysqld_path, "r")))
  {
    my_safe_printf_stderr("Error opening mysql binary\n");
    return;
  }
  if (fstat(fileno(mysqld), &statbuf) < 0)
  {
    my_safe_printf_stderr("Error running fstat on mysql binary\n");
    fclose(mysqld);
    return;
  }
  ehdr= (ElfW(Ehdr) *)my_mmap(0, statbuf.st_size, PROT_READ | PROT_WRITE,
                              MAP_PRIVATE, fileno(mysqld), 0);
  phdr= (ElfW(Phdr) *)(ehdr->e_phoff + (size_t)ehdr);
  while (phdr->p_type != PT_NOTE)
  {
    ++phdr;
  }
  nhdr= (ElfW(Nhdr) *)(phdr->p_offset + (size_t)ehdr);
  while (nhdr->n_type != NT_GNU_BUILD_ID)
  {
    nhdr= (ElfW(Nhdr) *)((size_t)nhdr + sizeof(ElfW(Nhdr)) + nhdr->n_namesz +
                         nhdr->n_descsz);
  }
  build_id= (unsigned char *)my_malloc(PSI_NOT_INSTRUMENTED, nhdr->n_descsz,
                                       MYF(0));
  memcpy(build_id,
         (void *)((size_t)nhdr + sizeof(ElfW(Nhdr)) + nhdr->n_namesz),
         nhdr->n_descsz);
  my_safe_printf_stderr("Build ID: ");
  for (i= 0; i < nhdr->n_descsz; ++i)
  {
    snprintf(buff, sizeof(buff), "%02hhx", build_id[i]);
    my_safe_printf_stderr("%s", buff);
  }
  my_free(build_id);
  fclose(mysqld);
  my_safe_printf_stderr("\n");
}
#endif  //defined(OS_LINUX)
void my_print_stacktrace(uchar* stack_bottom, ulong thread_stack)
{
  void *addrs[128];
  char **strings= NULL;
  int n = backtrace(addrs, array_elements(addrs));
  my_safe_printf_stderr("stack_bottom = %p thread_stack 0x%lx\n",
                        stack_bottom, thread_stack);
#if HAVE_ABI_CXA_DEMANGLE
  if ((strings= backtrace_symbols(addrs, n)))
  {
    my_demangle_symbols(strings, n);
    free(strings);
  }
#endif
  if (!strings)
  {
    backtrace_symbols_fd(addrs, n, fileno(stderr));
  }
}

#endif /* HAVE_PRINTSTACK || HAVE_BACKTRACE */
#endif /* HAVE_STACKTRACE */

#if HAVE_LIBCOREDUMPER
/* Produce a lib coredumper for the thread */
void my_write_libcoredumper(int sig, char *path, time_t curr_time)
{
  int        ret= 0;
  char       suffix[FN_REFLEN];
  char       core[FN_REFLEN];
  struct tm *timeinfo= gmtime(&curr_time);
  my_safe_printf_stderr("PATH: %s\n\n", path);
  if (path == NULL)
    strcpy(core, "core");
  else
    strcpy(core, path);
  sprintf(suffix, ".%d%02d%02d%02d%02d%02d", (1900 + timeinfo->tm_year),
          timeinfo->tm_mon, timeinfo->tm_mday, timeinfo->tm_hour,
          timeinfo->tm_min, timeinfo->tm_sec);
  strcat(core, suffix);
  ret= WriteCoreDump(core);
  if (ret != 0)
  {
    my_safe_printf_stderr("Error writting coredump: %d Signal: %d\n", ret,
                          sig);
  }
}
#endif
/* Produce a core for the thread */
void my_write_core(int sig)
{
  signal(sig, SIG_DFL);
  pthread_kill(my_thread_self(), sig);
#if defined(P_MYID)
  /* On Solaris, the above kill is not enough */
  sigsend(P_PID,P_MYID,sig);
#endif
}

#else /* _WIN32*/

#include <dbghelp.h>
#include <tlhelp32.h>
#if _MSC_VER
#pragma comment(lib, "dbghelp")
#endif

static EXCEPTION_POINTERS *exception_ptrs;

#define MODULE64_SIZE_WINXP 576
#define STACKWALK_MAX_FRAMES 64

void my_init_stacktrace()
{
}


void my_set_exception_pointers(EXCEPTION_POINTERS *ep)
{
  exception_ptrs = ep;
}

/*
  Appends directory to symbol path.
*/
static void add_to_symbol_path(char *path, size_t path_buffer_size, 
  char *dir, size_t dir_buffer_size)
{
  strcat_s(dir, dir_buffer_size, ";");
  if (!strstr(path, dir))
  {
    strcat_s(path, path_buffer_size, dir);
  }
}

/*
  Get symbol path - semicolon-separated list of directories to search for debug
  symbols. We expect PDB in the same directory as corresponding exe or dll,
  so the path is build from directories of the loaded modules. If environment
  variable _NT_SYMBOL_PATH is set, it's value appended to the symbol search path
*/
static void get_symbol_path(char *path, size_t size)
{ 
  HANDLE hSnap; 
  char *envvar;
  char *p;
#ifndef DBUG_OFF
  static char pdb_debug_dir[MAX_PATH + 7];
#endif

  path[0]= '\0';

#ifndef DBUG_OFF
  /* 
    Add "debug" subdirectory of the application directory, sometimes PDB will 
    placed here by installation.
  */
  GetModuleFileName(NULL, pdb_debug_dir, MAX_PATH);
  p= strrchr(pdb_debug_dir, '\\');
  if(p)
  { 
    *p= 0;
    strcat_s(pdb_debug_dir, sizeof(pdb_debug_dir), "\\debug;");
    add_to_symbol_path(path, size, pdb_debug_dir, sizeof(pdb_debug_dir));
  }
#endif

  /*
    Enumerate all modules, and add their directories to the path.
    Avoid duplicate entries.
  */
  hSnap= CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
  if (hSnap != INVALID_HANDLE_VALUE)
  {
    BOOL ret;
    MODULEENTRY32 mod;
    mod.dwSize= sizeof(MODULEENTRY32);
    for (ret= Module32First(hSnap, &mod); ret; ret= Module32Next(hSnap, &mod))
    {
      char *module_dir= mod.szExePath;
      p= strrchr(module_dir,'\\');
      if (!p)
      {
        /*
          Path separator was not found. Not known to happen, if ever happens,
          will indicate current directory.
        */
        module_dir[0]= '.';
        module_dir[1]= '\0';
      }
      else
      {
        *p= '\0';
      }
      add_to_symbol_path(path, size, module_dir,sizeof(mod.szExePath));
    }
    CloseHandle(hSnap);
  }

  
  /* Add _NT_SYMBOL_PATH, if present. */
  envvar= getenv("_NT_SYMBOL_PATH");
  if(envvar)
  {
    strcat_s(path, size, envvar);
  }
}

#define MAX_SYMBOL_PATH 32768

/* Platform SDK in VS2003 does not have definition for SYMOPT_NO_PROMPTS*/
#ifndef SYMOPT_NO_PROMPTS
#define SYMOPT_NO_PROMPTS 0
#endif

void my_print_stacktrace(uchar* unused1, ulong unused2)
{
  HANDLE  hProcess= GetCurrentProcess();
  HANDLE  hThread= GetCurrentThread();
  static  IMAGEHLP_MODULE64 module= {sizeof(module)};
  static  IMAGEHLP_SYMBOL64_PACKAGE package;
  DWORD64 addr;
  DWORD   machine;
  int     i;
  CONTEXT context;
  STACKFRAME64 frame={0};
  static char symbol_path[MAX_SYMBOL_PATH];

  if(!exception_ptrs)
    return;

  /* Copy context, as stackwalking on original will unwind the stack */
  context = *(exception_ptrs->ContextRecord);
  /*Initialize symbols.*/
  SymSetOptions(SYMOPT_LOAD_LINES|SYMOPT_NO_PROMPTS|SYMOPT_DEFERRED_LOADS|SYMOPT_DEBUG);
  get_symbol_path(symbol_path, sizeof(symbol_path));
  SymInitialize(hProcess, symbol_path, TRUE);

  /*Prepare stackframe for the first StackWalk64 call*/
  frame.AddrFrame.Mode= frame.AddrPC.Mode= frame.AddrStack.Mode= AddrModeFlat;
#if (defined _M_IX86)
  machine= IMAGE_FILE_MACHINE_I386;
  frame.AddrFrame.Offset= context.Ebp;
  frame.AddrPC.Offset=    context.Eip;
  frame.AddrStack.Offset= context.Esp;
#elif (defined _M_X64)
  machine = IMAGE_FILE_MACHINE_AMD64;
  frame.AddrFrame.Offset= context.Rbp;
  frame.AddrPC.Offset=    context.Rip;
  frame.AddrStack.Offset= context.Rsp;
#else
  /*There is currently no need to support IA64*/
#pragma error ("unsupported architecture")
#endif

  package.sym.SizeOfStruct= sizeof(package.sym);
  package.sym.MaxNameLength= sizeof(package.name);

  /*Walk the stack, output useful information*/ 
  for(i= 0; i< STACKWALK_MAX_FRAMES;i++)
  {
    DWORD64 function_offset= 0;
    DWORD line_offset= 0;
    IMAGEHLP_LINE64 line= {sizeof(line)};
    BOOL have_module= FALSE;
    BOOL have_symbol= FALSE;
    BOOL have_source= FALSE;

    if(!StackWalk64(machine, hProcess, hThread, &frame, &context, 0, 0, 0 ,0))
      break;
    addr= frame.AddrPC.Offset;

    have_module= SymGetModuleInfo64(hProcess,addr,&module);
#ifdef _M_IX86
    if(!have_module)
    {
      /*
        ModuleInfo structure has been "compatibly" extended in releases after XP,
        and its size was increased. To make XP dbghelp.dll function
        happy, pretend passing the old structure.
      */
      module.SizeOfStruct= MODULE64_SIZE_WINXP;
      have_module= SymGetModuleInfo64(hProcess, addr, &module);
    }
#endif

    have_symbol= SymGetSymFromAddr64(hProcess, addr, &function_offset,
      &(package.sym));
    have_source= SymGetLineFromAddr64(hProcess, addr, &line_offset, &line);

    my_safe_printf_stderr("%p    ", addr);
    if(have_module)
    {
      char *base_image_name= strrchr(module.ImageName, '\\');
      if(base_image_name)
        base_image_name++;
      else
        base_image_name= module.ImageName;
      my_safe_printf_stderr("%s!", base_image_name);
    }
    if(have_symbol)
      my_safe_printf_stderr("%s()", package.sym.Name);

    else if(have_module)
      my_safe_printf_stderr("%s", "???");

    if(have_source)
    {
      char *base_file_name= strrchr(line.FileName, '\\');
      if(base_file_name)
        base_file_name++;
      else
        base_file_name= line.FileName;
      my_safe_printf_stderr("[%s:%u]",
                            base_file_name, line.LineNumber);
    }
    my_safe_printf_stderr("%s", "\n");
  }
}


/*
  Write dump. The dump is created in current directory,
  file name is constructed from executable name plus
  ".dmp" extension
*/
void my_write_core(int unused)
{
  char path[MAX_PATH];
  char dump_fname[MAX_PATH]= "core.dmp";

  if(!exception_ptrs)
    return;

  if(GetModuleFileName(NULL, path, sizeof(path)))
  {
    _splitpath(path, NULL, NULL,dump_fname,NULL);
    strncat(dump_fname, ".dmp", sizeof(dump_fname));
  }
  my_create_minidump(dump_fname, 0, 0);
}


/** Create a minidump.
  @param name    path of minidump file.
  @param process HANDLE to process. (0 for own process).
  @param pid     Process id.
*/

void my_create_minidump(const char *name, HANDLE process, DWORD pid)
{
  char path[MAX_PATH];
  MINIDUMP_EXCEPTION_INFORMATION info;
  HANDLE hFile;

  if (process == 0)
  {
    /* Does not need to CloseHandle() for the below. */
    process= GetCurrentProcess();
    pid= GetCurrentProcessId();
    info.ExceptionPointers= exception_ptrs;
    info.ClientPointers= FALSE;
    info.ThreadId= GetCurrentThreadId();
  }

  hFile= CreateFile(name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS,
    FILE_ATTRIBUTE_NORMAL, 0);
  if(hFile)
  {
    MINIDUMP_TYPE mdt= (MINIDUMP_TYPE) (MiniDumpNormal |
                                        MiniDumpWithThreadInfo |
                                        MiniDumpWithProcessThreadData);
    /* Create minidump, use info only if same process. */
    if(MiniDumpWriteDump(process, pid, hFile, mdt,
                         process ? NULL : &info, 0, 0))
    {
      my_safe_printf_stderr("Minidump written to %s\n",
                            _fullpath(path, name, sizeof(path)) ?
                            path : name);
    }
    else
    {
      my_safe_printf_stderr("MiniDumpWriteDump() failed, last error %d\n",
                            GetLastError());
    }
    CloseHandle(hFile);
  }
  else
  {
    my_safe_printf_stderr("CreateFile(%s) failed, last error %d\n",
                          name, GetLastError());
  }
}


void my_safe_puts_stderr(const char *val, size_t len)
{
  __try
  {
    my_write_stderr(val, len);
    my_safe_printf_stderr("%s", "\n");
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {
    my_safe_printf_stderr("%s", "is an invalid string pointer\n");
  }
}
#endif /* _WIN32 */


#ifdef _WIN32
size_t my_write_stderr(const void *buf, size_t count)
{
  DWORD bytes_written;
  SetFilePointer(GetStdHandle(STD_ERROR_HANDLE), 0, NULL, FILE_END);
  WriteFile(GetStdHandle(STD_ERROR_HANDLE), buf, (DWORD)count, &bytes_written, NULL);
  return bytes_written;
}
#else
size_t my_write_stderr(const void *buf, size_t count)
{
  return (size_t) write(STDERR_FILENO, buf, count);
}
#endif


static const char digits[]= "0123456789abcdef";

char *my_safe_utoa(int base, ulonglong val, char *buf)
{
  *buf--= 0;
  do {
    *buf--= digits[val % base];
  } while ((val /= base) != 0);
  return buf + 1;
}


char *my_safe_itoa(int base, longlong val, char *buf)
{
  char *orig_buf= buf;
  const my_bool is_neg= (val < 0);
  *buf--= 0;

  if (is_neg)
    val= -val;
  if (is_neg && base == 16)
  {
    int ix;
    val-= 1;
    for (ix= 0; ix < 16; ++ix)
      buf[-ix]= '0';
  }
  
  do {
    *buf--= digits[val % base];
  } while ((val /= base) != 0);

  if (is_neg && base == 10)
    *buf--= '-';

  if (is_neg && base == 16)
  {
    int ix;
    buf= orig_buf - 1;
    for (ix= 0; ix < 16; ++ix, --buf)
    {
      switch (*buf)
      {
      case '0': *buf= 'f'; break;
      case '1': *buf= 'e'; break;
      case '2': *buf= 'd'; break;
      case '3': *buf= 'c'; break;
      case '4': *buf= 'b'; break;
      case '5': *buf= 'a'; break;
      case '6': *buf= '9'; break;
      case '7': *buf= '8'; break;
      case '8': *buf= '7'; break;
      case '9': *buf= '6'; break;
      case 'a': *buf= '5'; break;
      case 'b': *buf= '4'; break;
      case 'c': *buf= '3'; break;
      case 'd': *buf= '2'; break;
      case 'e': *buf= '1'; break;
      case 'f': *buf= '0'; break;
      }
    }
  }
  return buf+1;
}


static const char *check_longlong(const char *fmt, my_bool *have_longlong)
{
  *have_longlong= FALSE;
  if (*fmt == 'l')
  {
    fmt++;
    if (*fmt != 'l')
      *have_longlong= (sizeof(long) == sizeof(longlong));
    else
    {
      fmt++;
      *have_longlong= TRUE;
    }
  }
  return fmt;
}

static size_t my_safe_vsnprintf(char *to, size_t size,
                                const char* format, va_list ap)
{
  char *start= to;
  char *end= start + size - 1;
  for (; *format; ++format)
  {
    my_bool have_longlong = FALSE;
    if (*format != '%')
    {
      if (to == end)                            /* end of buffer */
        break;
      *to++= *format;                           /* copy ordinary char */
      continue;
    }
    ++format;                                   /* skip '%' */

    format= check_longlong(format, &have_longlong);

    switch (*format)
    {
    case 'd':
    case 'i':
    case 'u':
    case 'x':
    case 'p':
      {
        longlong ival= 0;
        ulonglong uval = 0;
        if (*format == 'p')
          have_longlong= (sizeof(void *) == sizeof(longlong));
        if (have_longlong)
        {
          if (*format == 'u')
            uval= va_arg(ap, ulonglong);
          else
            ival= va_arg(ap, longlong);
        }
        else
        {
          if (*format == 'u')
            uval= va_arg(ap, unsigned int);
          else
            ival= va_arg(ap, int);
        }

        {
          char buff[22];
          const int base= (*format == 'x' || *format == 'p') ? 16 : 10;
          char *val_as_str= (*format == 'u') ?
            my_safe_utoa(base, uval, &buff[sizeof(buff)-1]) :
            my_safe_itoa(base, ival, &buff[sizeof(buff)-1]);

          /*
            Strip off "ffffffff" if we have 'x' format without 'll'
            Similarly for 'p' format on 32bit systems.
          */
          if (base == 16 && !have_longlong && ival < 0)
            val_as_str+= 8;

          while (*val_as_str && to < end)
            *to++= *val_as_str++;
          continue;
        }
      }
    case 's':
      {
        const char *val= va_arg(ap, char*);
        if (!val)
          val= "(null)";
        while (*val && to < end)
          *to++= *val++;
        continue;
      }
    }
  }
  *to= 0;
  return to - start;
}


size_t my_safe_snprintf(char* to, size_t n, const char* fmt, ...)
{
  size_t result;
  va_list args;
  va_start(args,fmt);
  result= my_safe_vsnprintf(to, n, fmt, args);
  va_end(args);
  return result;
}


size_t my_safe_printf_stderr(const char* fmt, ...)
{
  char to[512];
  size_t result;
  va_list args;
  va_start(args,fmt);
  result= my_safe_vsnprintf(to, sizeof(to), fmt, args);
  va_end(args);
  my_write_stderr(to, result);
  return result;
}


void my_safe_print_system_time()
{
  char hrs_buf[3]= "00";
  char mins_buf[3]= "00";
  char secs_buf[3]= "00";
  int base= 10;
#ifdef _WIN32
  SYSTEMTIME utc_time;
  long hrs, mins, secs;
  GetSystemTime(&utc_time);
  hrs=  utc_time.wHour;
  mins= utc_time.wMinute;
  secs= utc_time.wSecond;
#else
  /* Using time() instead of my_time() to avoid looping */
  const time_t curr_time= time(NULL);
  /* Calculate time of day */
  const long tmins = curr_time / 60;
  const long thrs  = tmins / 60;
  const long hrs   = thrs  % 24;
  const long mins  = tmins % 60;
  const long secs  = curr_time % 60;
#endif

  my_safe_itoa(base, hrs, &hrs_buf[2]);
  my_safe_itoa(base, mins, &mins_buf[2]);
  my_safe_itoa(base, secs, &secs_buf[2]);

  my_safe_printf_stderr("---------- %s:%s:%s UTC - ",
                        hrs_buf, mins_buf, secs_buf);
}

