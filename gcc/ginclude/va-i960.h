/* GNU C varargs support for the Intel 80960.  */

/* Define __gnuc_va_list.  */

#ifndef __GNUC_VA_LIST
#define __GNUC_VA_LIST
/* The first element is the address of the first argument.
   The second element is the number of bytes skipped past so far.  */
typedef unsigned __gnuc_va_list[2];	
#endif /* not __GNUC_VA_LIST */

/* If this is for internal libc use, don't define anything but
   __gnuc_va_list.  */
#if defined (_STDARG_H) || defined (_VARARGS_H)

/* In GCC version 2, we want an ellipsis at the end of the declaration
   of the argument list.  GCC version 1 can't parse it.  */

#if __GNUC__ > 1
#define __va_ellipsis ...
#else
#define __va_ellipsis
#endif

/* The stack size of the type t.  */
#define __vsiz(T)   (((sizeof (T) + 3) / 4) * 4)
/* The stack alignment of the type t.  */
#define __vali(T)   (__alignof__ (T) >= 4 ? __alignof__ (T) : 4)
/* The offset of the next stack argument after one of type t at offset i.  */
#define __vpad(I, T) ((((I) + __vali (T) - 1) / __vali (T)) \
		       * __vali (T) + __vsiz (T))

#ifdef _STDARG_H
#define va_start(AP, LASTARG)				\
__extension__						\
({ __asm__ ("st	g14,%0" : "=m" (*(AP)));		\
   (AP)[1] = (unsigned) __builtin_next_arg () - *AP; })
#else

#define	va_alist __builtin_va_alist
#define	va_dcl	 char *__builtin_va_alist; __va_ellipsis
#define	va_start(AP) ((AP)[1] = 0, *(AP) = (unsigned) &va_alist)
#endif

/* We cast to void * and then to TYPE * because this avoids
   a warning about increasing the alignment requirement.  */
#define	va_arg(AP, T)							\
(									\
  (									\
    ((AP)[1] <= 48 && (__vpad ((AP)[1], T) > 48 || __vsiz (T) > 16))	\
      ? ((AP)[1] = 48 + __vsiz (T))					\
      : ((AP)[1] = __vpad ((AP)[1], T))					\
  ),									\
									\
  *((T *) (void *) ((char *) *(AP) + (AP)[1] - __vsiz (T)))		\
)

void va_end (__gnuc_va_list);		/* Defined in libgcc.a */
#define	va_end(AP)

#endif /* defined (_STDARG_H) || defined (_VARARGS_H) */

