/* Operating system specific defines to be used when targeting GCC for
   hosting on Windows NT 3.x, using a Unix style C library and tools,
   as distinct from winnt.h, which is used to build GCC for use with a
   windows style library and tool set and uses the Microsoft tools.
   Copyright (C) 1995, 1996, 1997, 1998, 1999, 2000, 2002
   Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* Enable parsing of #pragma pack(push,<n>) and #pragma pack(pop).  */
#define HANDLE_PRAGMA_PACK_PUSH_POP 1

#define DBX_DEBUGGING_INFO 
#define SDB_DEBUGGING_INFO 
#define PREFERRED_DEBUGGING_TYPE DBX_DEBUG

#include "i386/unix.h"
#include "i386/bsd.h"
#include "i386/gas.h"
#include "dbxcoff.h"

/* Augment TARGET_SWITCHES with the cygwin/win32 options.  */
#define MASK_WIN32 0x40000000 /* Use -lming32 interface */
#define MASK_CYGWIN  0x20000000 /* Use -lcygwin interface */
#define MASK_WINDOWS 0x10000000 /* Use windows interface */
#define MASK_DLL     0x08000000 /* Use dll interface    */
#define MASK_NOP_FUN_DLLIMPORT 0x20000 /* Ignore dllimport for functions */

#define TARGET_WIN32             (target_flags & MASK_WIN32)
#define TARGET_CYGWIN            (target_flags & MASK_CYGWIN)
#define TARGET_WINDOWS           (target_flags & MASK_WINDOWS)
#define TARGET_DLL               (target_flags & MASK_DLL)
#define TARGET_NOP_FUN_DLLIMPORT (target_flags & MASK_NOP_FUN_DLLIMPORT)

#undef  SUBTARGET_SWITCHES
#define SUBTARGET_SWITCHES						\
    { "win32",			MASK_WIN32,				\
      N_("Use Mingw32 interface") },					\
    { "cygwin",			MASK_CYGWIN,				\
      N_("Use Cygwin interface")  },					\
    { "windows",		MASK_WINDOWS,				\
      N_("Use bare Windows interface") },				\
    { "dll",			MASK_DLL,				\
      N_("Generate code for a DLL") },					\
    { "nop-fun-dllimport",	MASK_NOP_FUN_DLLIMPORT,			\
      N_("Ignore dllimport for functions") }, 				\
    { "no-nop-fun-dllimport",	MASK_NOP_FUN_DLLIMPORT, "" },


#define TARGET_OS_CPP_BUILTINS()					\
  do									\
    {									\
	builtin_define ("_WIN32");					\
	builtin_define_std ("WINNT");					\
	builtin_define ("_X86_");					\
	builtin_define ("__stdcall=__attribute__((__stdcall__))");	\
	builtin_define ("__cdecl=__attribute__((__cdecl__))");		\
	builtin_assert ("system=winnt");				\
	if (TARGET_CYGWIN)						\
	  {								\
	    builtin_define ("__CYGWIN32__");				\
	    builtin_define ("__CYGWIN__");				\
	  }								\
	else								\
	  builtin_define ("__MINGW32__");				\
    }									\
  while (0)

#undef STARTFILE_SPEC

#define STARTFILE_SPEC "%{mdll:dllcrt0%O%s} %{!mdll: %{!mcygwin:mcrt0%O%s} \
                        %{mcygwin:crt0%O%s} %{pg:gcrt0%O%s}}"

#undef CPP_SPEC
#define CPP_SPEC "%{posix:-D_POSIX_SOURCE} \
  %{!mcygwin:-iwithprefixbefore include/mingw32}"

/* We have to dynamic link to get to the system DLLs.  All of libc, libm and
   the Unix stuff is in cygwin.dll.  The import library is called
   'libcygwin.a'.  For Windows applications, include more libraries, but
   always include kernel32.  We'd like to specific subsystem windows to
   ld, but that doesn't work just yet.  */

#undef LIB_SPEC
#define LIB_SPEC "%{pg:-lgmon}						\
		  %{!mcygwin:-lmingw32 -lmoldname -lmsvcrt -lcrtdll}	\
                  %{mcygwin:-lcygwin} %{mwindows:-luser32 -lgdi32 -lcomdlg32} \
                  -lkernel32 -ladvapi32 -lshell32"

#define LINK_SPEC "%{mwindows:--subsystem windows} \
  %{mdll:--dll -e _DllMainCRTStartup@12}"

#define SIZE_TYPE "unsigned int"
#define PTRDIFF_TYPE "int"
#define WCHAR_TYPE_SIZE 16
#define WCHAR_TYPE "short unsigned int"
/* Currently we do not have the atexit() function,
   so take that from libgcc2.c */

#define NEED_ATEXIT 1

#undef TARGET_ENCODE_SECTION_INFO
#define TARGET_ENCODE_SECTION_INFO  i386_pe_encode_section_info
#undef  TARGET_STRIP_NAME_ENCODING
#define TARGET_STRIP_NAME_ENCODING  i386_pe_strip_name_encoding_full

/* Emit code to check the stack when allocating more that 4000
   bytes in one go.  */

#define CHECK_STACK_LIMIT 4000

/* By default, target has a 80387, uses IEEE compatible arithmetic,
   and returns float values in the 387 and needs stack probes */
#undef TARGET_SUBTARGET_DEFAULT

#define TARGET_SUBTARGET_DEFAULT \
   (MASK_80387 | MASK_IEEE_FP | MASK_FLOAT_RETURNS | MASK_STACK_PROBE) 

/* This is how to output an assembler line
   that says to advance the location counter
   to a multiple of 2**LOG bytes.  */

#undef ASM_OUTPUT_ALIGN
#define ASM_OUTPUT_ALIGN(FILE,LOG)	\
    if ((LOG)!=0) fprintf ((FILE), "\t.align %d\n", 1<<(LOG))

/* Define this macro if in some cases global symbols from one translation
   unit may not be bound to undefined symbols in another translation unit
   without user intervention.  For instance, under Microsoft Windows
   symbols must be explicitly imported from shared libraries (DLLs).  */
#define MULTIPLE_SYMBOL_SPACES

extern void i386_pe_unique_section PARAMS ((tree, int));
#define TARGET_ASM_UNIQUE_SECTION i386_pe_unique_section

#define SUPPORTS_ONE_ONLY 1

/* Switch into a generic section.  */
#define TARGET_ASM_NAMED_SECTION  i386_pe_asm_named_section

/* Select attributes for named sections.  */
#define TARGET_SECTION_TYPE_FLAGS  i386_pe_section_type_flags

#undef ASM_COMMENT_START
#define ASM_COMMENT_START " #"

/* Don't assume anything about the header files.  */
#define NO_IMPLICIT_EXTERN_C

#define SUBTARGET_PROLOGUE						\
  if (current_function_profile						\
      && MAIN_NAME_P (DECL_NAME (current_function_decl))		\
     {									\
      rtx xops[1];							\
      xops[0] = gen_rtx_MEM (FUNCTION_MODE,				\
			 gen_rtx (SYMBOL_REF, Pmode, "_monstartup"));	\
      emit_call_insn (gen_rtx (CALL, VOIDmode, xops[0], const0_rtx));	\
     }
