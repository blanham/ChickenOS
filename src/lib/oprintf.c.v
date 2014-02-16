/*----------------------------------------------------------------------------
Stripped-down printf()
Chris Giese	<geezer@execpc.com>	http://my.execpc.com/~geezer

I, the copyright holder of this work, hereby release it into the
public domain. This applies worldwide. If this is not legally possible:
I grant any entity the right to use this work for any purpose,
without any conditions, unless such conditions are required by law.

Revised Dec 3, 2013
- do_printf() restructured to get rid of confusing goto statements
- do_printf() now returns EOF if an error occurs
  (currently, this happens only if out-of-memory in vasprintf_help())
- added vasprintf() and asprintf()
- added support for %Fs (far pointer to string)
- compile-time option (PRINTF_UCP) to display pointer values
  as upper-case hex (A-F), instead of lower-case (a-f)
- the code to check for "%--6d", "%---6d", etc. has been removed;
  these are now treated the same as "%-6d"

Revised Feb 3, 2008
- sprintf() now works with NULL buffer; returns size of output
- changed va_start() macro to make it compatible with ANSI C

Revised Dec 12, 2003
- fixed vsprintf() and sprintf() in test code

Revised Jan 28, 2002
- changes to make characters 0x80-0xFF display properly

Revised June 10, 2001
- changes to make vsprintf() terminate string with '\0'

Revised May 12, 2000
- math in DO_NUM (currently num2asc()) is now unsigned, as it should be
- %0 flag (pad left with zeroes) now works
- actually did some TESTING, maybe fixed some other bugs

%[flag][width][.prec][mod][conv]
flag:	-	left justify, pad right w/ blanks	DONE
	0	pad left w/ 0 for numerics		DONE
	+	always print sign, + or -		no
	' '	(blank)					no
	#	(???)					no

width:		(field width)				DONE

prec:		(precision)				no

conv:	f,e,g,E,G float					no
	d,i	decimal int				DONE
	u	decimal unsigned			DONE
	o	octal					DONE
	x,X	hex					DONE
	c	char					DONE
	s	string					DONE
	p	ptr					DONE

mod:	N	near ptr				DONE
	F	far ptr					%Fs only; not %Fp
	h	short (16-bit) int			DONE
	l	long (32-bit) int			DONE
	L	long long (64-bit) int			no

To do:
- implement '+' flag
- implement ' ' flag
- implement '%Fp'
- implement specifier (finally!)
- support floating point (finally!)
- int vsnprintf(char *str, size_t size, const char *format, va_list ap);
- int snprintf(char *str, size_t size, const char *format, ...);
- int vfprintf(FILE *stream, const char *format, va_list ap);
- int fprintf(FILE *stream, const char *format, ...);
----------------------------------------------------------------------------*/
#include <string.h> /* strlen() */
#include <stdio.h> /* EOF */

#if 1
#include <stdarg.h> /* va_list, va_start(), va_arg(), va_end() */

#else
/* Home-brew STDARG.H, also public-domain. WARNING: This code won't
work with register calling convention such as 64-bit x86 ABI. */

/* Assume: width of stack == width of int. Don't use sizeof(char *) or
other pointer because sizeof(char *)==4 for LARGE-model 16-bit code.
Assume: width is a power of 2 */
#define	STACK_WIDTH	sizeof(int)

/* Round up object width so it's an even multiple of STACK_WIDTH.
Using AND for modulus here, so STACK_WIDTH must be a power of 2. */
#define	TYPE_WIDTH(TYPE)				\
	((sizeof(TYPE) + STACK_WIDTH - 1) & ~(STACK_WIDTH - 1))

/* point the va_list pointer to LASTARG,
then advance beyond it to the first variable arg */
#define	va_start(PTR, LASTARG)				\
	PTR = (va_list)((char *)&(LASTARG) + TYPE_WIDTH(LASTARG))

/* Increment the va_list pointer, then "return"
(evaluate to, actually) the previous value of the pointer. */
#define va_arg(PTR, TYPE)	(			\
	PTR = (char *)(PTR) + TYPE_WIDTH(TYPE)		\
				,			\
	*((TYPE *)((char *)(PTR) - TYPE_WIDTH(TYPE)))	\
				)

#define va_end(PTR)	/* nothing */

typedef void *va_list;

#endif

#if defined(__TURBOC__)||defined(__WATCOMC__)
/* support printf("%Fs", ...) */
#define	PRINTF_FS	1
#endif

/* display pointers in upper-case hex (A-F) instead of lower-case (a-f) */
#define	PRINTF_UCP	1

/* flags used in processing format string */
#define	PR_LEFTJUST	0x01	/* left justify */
#define	PR_UC		0x02	/* use upper case (A-F vs. a-f) for hex */
#define	PR_DO_SIGN	0x04	/* signed numeric conversion (%d vs. %u) */
#define	PR_LONG		0x08	/* long numeric conversion (%lu) */
#define	PR_SHORT	0x10	/* short numeric conversion (%hu) */
#define	PR_NEGATIVE	0x20	/* PR_DO_SIGN set and num was < 0 */
#define	PR_PADLEFT0	0x40	/* pad left with '0' instead of ' ' */
#define	PR_FARPTR	0x80	/* pointers are far */

typedef int (*fnptr_t)(unsigned c, void **helper);
/*****************************************************************************
Converts binary numeric value 'num' to ASCII string.
'buf' must point to a zeroed byte at the end of a suitably-large buffer.
Return value is a pointer to the START of the string stored in 'buf'.
*****************************************************************************/
#if defined(PRINTF_FS)
static unsigned char far *num2asc(unsigned char far *buf, unsigned radix,
		unsigned long num, int upper_case)
#else
static unsigned char *num2asc(unsigned char *buf, unsigned radix,
		unsigned long num, int upper_case)
#endif
{
	unsigned long i;

/* do..while() instead of while() ensures we get a single '0' if num==0 */
	do
	{
		i = (unsigned long)num % radix;
		buf--;
		if(i < 10)
			*buf = i + '0';
		else
		{
			if(upper_case)
				*buf = i - 10 + 'A';
			else
				*buf = i - 10 + 'a';
		}
		num = (unsigned long)num / radix;
	} while(num != 0);
	return buf;
}
/****************************************************************************/
#if defined(__TURBOC__)
#if __TURBOC__==0x296 /* Turbo C++ 1.01 */
size_t far _fstrlen(const char far *s)
{
	size_t rv = 0;

	for(; *s != '\0'; s++)
		rv++;
	return rv;
}
#endif
#endif
/*****************************************************************************
name:	do_printf
action:	minimal subfunction for ?printf, calls function
	'fn' with arg 'ptr' for each character to be output
returns:total number of characters output
*****************************************************************************/
#include <stdlib.h> /* exit() */

static int do_printf(const char *fmt, va_list args, fnptr_t fn, void *ptr)
{
	unsigned flags, actual_wd, given_wd, count;
/* largest number handled is 2^32-1, lowest radix handled is 8.
2^32-1 in base 8 has 11 digits (add 5 for trailing NUL and for slop) */
#define	PR_BUFLEN	16
	unsigned char state, radix, buf[PR_BUFLEN];
#if defined(PRINTF_FS)
	unsigned char far *where;
#else
	unsigned char *where;
#endif
	long num;
	int i;

	state = count = given_wd = flags = 0;
/* having the for() and the switch() on the same line looks jarring
but the indentation gets out of hand otherwise */
	for(; *fmt; fmt++) switch(state)
	{
/* STATE 0: AWAITING '%' */
	case 0:
/* echo text until '%' seen */
		if(*fmt != '%')
		{
			if((i = fn(*fmt, &ptr)) == EOF)
				return i;
			count++;
			break;
		}
/* found %, get next char and advance state to check if next char is a flag */
		fmt++;
		state++;
		flags = 0;
		/* FALL THROUGH */
/* STATE 1: AWAITING FLAGS ('%', '-', or '0') */
	case 1:
		if(*fmt == '%')	/* %% */
		{
			if((i = fn(*fmt, &ptr)) == EOF)
				return i;
			count++;
			state = 0;
			break;
		}
		if(*fmt == '-')
		{
			flags |= PR_LEFTJUST;
			break;
		}
		if(*fmt == '0')
		{
			flags |= PR_PADLEFT0;
/* '0' could be flag or field width -- fall through */
			fmt++;
		}
/* '0' or not a flag char: advance state to check if it's field width */
		state++;
		given_wd = 0;
		/* FALL THROUGH */
/* STATE 2: AWAITING (NUMERIC) FIELD WIDTH */
	case 2:
		if(*fmt >= '0' && *fmt <= '9')
		{
			given_wd = 10 * given_wd + (*fmt - '0');
			break;
		}
/* not field width: advance state to check if it's a modifier */
		state++;
		/* FALL THROUGH */
/* STATE 3: AWAITING MODIFIER CHARACTERS ('F', 'N', 'l', or 'h') */
	case 3:
		if(*fmt == 'F')
		{
			flags |= PR_FARPTR;
			break;
		}
		if(*fmt == 'N') /* near pointer (the default) */
		{
			flags &= ~PR_FARPTR;
			break;
		}
		if(*fmt == 'l')
		{
			flags |= PR_LONG;
			break;
		}
		if(*fmt == 'h')
		{
			flags |= PR_SHORT;
			break;
		}
/* not a modifier: advance state to check if it's a conversion char */
		state++;
		/* FALL THROUGH */
/* STATE 4: AWAITING CONVERSION CHARACTER
('X', 'x', 'p', 'n', 'd', 'i', 'u', 'o', 'c', or 's') */
	case 4:
		where = &buf[PR_BUFLEN - 1];
		*where = '\0';
/* 'h' and 'l' modifiers cancel each other out */
		if((flags & (PR_LONG | PR_SHORT)) == (PR_LONG | PR_SHORT))
			flags &= ~(PR_LONG | PR_SHORT);
/* pointer and numeric conversions */
		if(*fmt == 'p' || *fmt == 'n'
			|| *fmt == 'X' || *fmt == 'x'
			|| *fmt == 'd' || *fmt == 'i'
			|| *fmt == 'u' || *fmt == 'o')
		{
#if defined(PRINTF_UCP)
			if(*fmt == 'X' || *fmt == 'p' || *fmt == 'n')
#else
			if(*fmt == 'X')
#endif
			{
				flags |= PR_UC;
				flags &= ~PR_DO_SIGN;
				radix = 16;
			}
#if defined(PRINTF_UCP)
			else if(*fmt == 'x')
#else
			else if(*fmt == 'x' || *fmt == 'p' || *fmt == 'n')
#endif
			{
				flags &= ~PR_UC;
				flags &= ~PR_DO_SIGN;
				radix = 16;
			}
			else if(*fmt == 'd' || *fmt == 'i')
			{
				flags |= PR_DO_SIGN;
				radix = 10;
			}
			else if(*fmt == 'u')
			{
				flags &= ~PR_DO_SIGN;
				radix = 10;
			}
			else /*if(*fmt == 'o')*/
			{
				flags &= ~PR_DO_SIGN;
				radix = 8;
			}
/* load the value to be printed. 'l'=long (usu. 32 bits)... */
			if(flags & PR_LONG)
			{
				if(flags & PR_DO_SIGN)
					num = va_arg(args, signed long);
				else
					num = va_arg(args, unsigned long);
			}
/* ...'h'=short (usu. 16 bits) */
			else if(flags & PR_SHORT)
			{
				if(flags & PR_DO_SIGN)
					num = va_arg(args, signed short);
				else
					num = va_arg(args, unsigned short);
			}
/* no 'h' nor 'l': sizeof(int) bits */
			else
			{
				if(flags & PR_DO_SIGN)
					num = va_arg(args, signed int);
				else
					num = va_arg(args, unsigned int);
			}
/* convert to unsigned but remember that it was negative */
			if((flags & PR_DO_SIGN) && num < 0)
			{
				flags |= PR_NEGATIVE;
				num = -num;
			}
/* convert binary unsigned number to octal/decimal/hex ASCII */
			where = num2asc(where, radix, num, flags & PR_UC);
		}
		else if(*fmt == 'c')
		{
/* disallow these modifiers for %c: do sign, negative, pad left with 0 */
			flags &= ~(PR_DO_SIGN | PR_NEGATIVE | PR_PADLEFT0);
/* yes; we're converting a character to a string here: */
			where--;
			*where = (unsigned char)va_arg(args, unsigned char);
		}
		else if(*fmt == 's')
		{
/* disallow these modifiers for %s: do sign, negative, pad left with 0 */
			flags &= ~(PR_DO_SIGN | PR_NEGATIVE | PR_PADLEFT0);
#if defined(PRINTF_FS)
			if(flags & PR_FARPTR)
				where = va_arg(args, unsigned char far *);
			else
#endif
				where = va_arg(args, unsigned char *);
		}
/* bogus conversion character -- copy it to output and go back to state 0 */
		else
		{
			if((i = fn(*fmt, &ptr)) == EOF)
				return i;
			count++;
			state = flags = given_wd = 0;
			break;
		}
/* emit formatted string */
#if defined(PRINTF_FS)
		actual_wd = _fstrlen(where);
#else
		actual_wd = strlen(where);
#endif
		if(flags & PR_NEGATIVE)
			actual_wd++;
/* if we pad left with ZEROES, do the sign now
(for numeric values; not for %c or %s) */
		if((flags & (PR_NEGATIVE | PR_PADLEFT0)) == (PR_NEGATIVE | PR_PADLEFT0))
		{
			if((i = fn('-', &ptr)) == EOF)
				return i;
			count++;
		}
/* pad on left with spaces or zeroes (for right justify) */
		if((flags & PR_LEFTJUST) == 0)
		{
			for(; given_wd > actual_wd; given_wd--)
			{
				if((i = fn(flags & PR_PADLEFT0
					? '0' : ' ', &ptr)) == EOF)
						return i;
				count++;
			}
/* if we pad left with SPACES, do the sign now */
			if(flags & PR_NEGATIVE)
			{
				if((i = fn('-', &ptr)) == EOF)
					return i;
				count++;
			}
		}
/* emit converted number/char/string */
		for(; *where != '\0'; where++)
		{
			if((i = fn(*where, &ptr)) == EOF)
				return i;
			count++;
		}
/* pad on right with spaces (for left justify) */
		if(given_wd < actual_wd)
			given_wd = 0;
		else
			given_wd -= actual_wd;
		for(; given_wd; given_wd--)
		{
			if((i = fn(' ', &ptr)) == EOF)
				return i;
			count++;
		}
		/* FALL THROUGH */
	default:
		state = flags = given_wd = 0;
		break;
	}
	return count;
}
#if 1 /* testing */
/****************************************************************************/
static int vsprintf_help(unsigned c, void **ptr)
{
	char *dst;

	dst = *ptr;
	*dst++ = (char)c;
	*ptr = dst;
	return 0 ;
}
/****************************************************************************/
int vsprintf(char *buf, const char *fmt, va_list args)
{
	int rv;

	rv = do_printf(fmt, args, vsprintf_help, (void *)buf);
	buf[rv] = '\0';
	return rv;
}
/****************************************************************************/
static int discard(unsigned c_UNUSED, void **ptr_UNUSED)
{
	return 0;
}
/****************************************************************************/
int sprintf(char *buf, const char *fmt, ...)
{
	va_list args;
	int rv;

	va_start(args, fmt);
/* In lieu of snprintf(), some C libraries let you call sprintf() with a
NULL buffer to determine how many bytes of output will be generated.
Then again, if you're writing your own printf(), it's probably just
as easy to write your own snprintf()... */
	if(buf == NULL)
		rv = do_printf(fmt, args, discard, NULL);
	else
		rv = vsprintf(buf, fmt, args);
	va_end(args);
	return rv;
}
/****************************************************************************/
#include <stdlib.h> /* realloc(), free() */

typedef struct
{
	unsigned char *buf;
	unsigned alloc, ptr;
} asprintf_t;

static int vasprintf_help(unsigned c, void **ptr)
{
	unsigned char *t;
	asprintf_t *as;
	unsigned len;

	as = *ptr;
	if(as->ptr + 1 >= as->alloc)
	{
/* start with 16 bytes, then double the string length every time it
overflows */
		len = (as->alloc == 0) ? 16 : (as->alloc * 2);
		t = realloc(as->buf, len);
		if(t == NULL)
			return EOF;
		as->buf = t;
		as->alloc = len;
	}
	as->buf[as->ptr] = c;
	as->ptr++;
	return 0 ;
}
/****************************************************************************/
int vasprintf(char **dst_ptr, const char *fmt, va_list args)
{
	asprintf_t as;
	int i;

	as.buf = NULL;
	as.alloc = as.ptr = 0;
	i = do_printf(fmt, args, vasprintf_help, &as);
	if(i == EOF)
	{
		if(as.buf != NULL)
			free(as.buf);
		*dst_ptr = NULL; /* FreeBSD does this */
		return -1;
	}
	as.buf[as.ptr] = '\0';
	*dst_ptr = as.buf;
	return i;
}
/******************************************************************************
No buffer overflow here, hopefully -- a buffer of sufficient size
should be allocated by vasprintf_help(). The value returned via dst_ptr
should be free()d when no longer needed.
******************************************************************************/
int asprintf(char **dst_ptr, const char *fmt, ...)
{
	va_list args;
	int rv;

	va_start(args, fmt);
	rv = vasprintf(dst_ptr, fmt, args);
	va_end(args);
	return rv;
}
/****************************************************************************/
#include <stdio.h> /* putchar() */

int vprintf_help(unsigned c, void **ptr_UNUSED)
{
	putchar(c);
	return 0 ;
}
/****************************************************************************/
int vprintf(const char *fmt, va_list args)
{
	return do_printf(fmt, args, vprintf_help, NULL);
}
/****************************************************************************/
int printf(const char *fmt, ...)
{
	va_list args;
	int rv;

	va_start(args, fmt);
	rv = vprintf(fmt, args);
	va_end(args);
	return rv;
}
/****************************************************************************/
#include <stdio.h> /* stdout, fputs() (but not printf() :) */
#if defined(PRINTF_FS)
#include <dos.h> /* MK_FP() */
#endif

int main(void)
{
#if defined(PRINTF_FS)
#pragma pack(1)
	static struct
	{
/*   0 */	char res1[6];
/*   6 */	char far *oem_name;
/*  20 */	char res2[502];
/* 512 */} vbe_info;
/**/
	struct SREGS sregs;
	union REGS regs;
#endif
	char buf[64], *foo;
	int i;

	sprintf(buf, "%u score and %i years ago...\n", 4, -7);
	fputs(buf, stdout); /* puts() adds newline */

	sprintf(buf, "-1L == 0x%lX == octal %lo\n", -1L, -1L);
	fputs(buf, stdout);

	printf("<%-8s> and <%8s> justified strings\n", "left", "right");

#if defined(__MSDOS__)||defined(__DOS__)||defined(__WIN32__) /* I think... */
	printf("'GrÅn' is German for 'green'\n"); /* CP437 */
#else
	printf("'Gr¸n' is German for 'green'\n"); /* Latin-1 */
#endif
	i = sprintf(NULL, "-1L == hex %lX == octal %lo\n", -1L, -1L);
	printf("Counting the final zero byte, the following string "
		"is %u bytes long:\n", i);
	printf("-1L == hex %lX == octal %lo\n", -1L, -1L);


	printf(	"Hex                      Mem-\n"
		"mode           Bits Gran ory Hex\n"
		"num-           per  size mod banked\n"
		"ber  Wd   Ht   pix. (K)  -el FB addr\n"
		"---- ---- ---- ---- ---- --- --------\n");
	printf("%4X %4u %4u %4u %4u %3u %8lX\n",
		0x100, 640, 400, 8, 64, 4, 0xA0000L);
#if defined(PRINTF_FS)
	regs.x.ax = 0x4F00;
	sregs.es = FP_SEG(&vbe_info);
	regs.x.di = FP_OFF(&vbe_info);
	int86x(0x10, &regs, &regs, &sregs);
	if(regs.x.ax == 0x004F)
		printf("VBE OEM name is '%Fs'\n", vbe_info.oem_name);
#endif


/* there are 16 percents so 8 should be printed */
	printf("%%%%%%%%%%%%%%%%\n");
/* the extra '-'s are now ignored */
	printf("<%---------8d>\n", 42);
/* extra '0's are ignored */
	printf("<%0000000008d>\n", 42);
/* 16-bit value displayed here for 16-bit compiler, 32-bit for 32-bit */
	printf("0x%hlx\n", 0xDEADBEEFL);
	printf("0x%lhX\n", 0xDEADBEEFL);
	printf("0x%hhhlllhlhllhlhhhhhhhhhhx\n", 0xDEADBEEFL);
	printf("0x%x\n", 0xDEADBEEFL);
/* 16-bit value always */
	printf("0x%hX\n", 0xDEADBEEFL);
/* 32-bit value always */
	printf("0x%lx\n", 0xDEADBEEFL);
/* 16-bit value displayed here for 16-bit compiler, 32-bit for 32-bit */
	printf("%p\n", (char *)0xDEADBEEFL);
/* '%z' is not valid -- everything from 'z' forward is printed */
	printf("%zebras are black and white\n");

	foo = NULL;
	i = asprintf(&foo, "__DATE__=%s, __TIME__=%s, __FILE__=%s, __LINE__=%u\n",
		__DATE__, __TIME__, __FILE__, __LINE__);
	if(i > 0)
		puts(foo);
	else if(i <= 0)
		printf("Error: asprintf() returned %d\n", i);
	if(foo != NULL)
		free(foo);
	return 0;
}
#endif
