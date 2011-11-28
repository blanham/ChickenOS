#include <kernel/console.h>
#include <stdio.h>
#include <stdarg.h>
#define NULL 0
#define ALLOCSIZE 10000
static char allocbuf[ALLOCSIZE];
static char *allocp = allocbuf;
/* temporary memory functions taken from K&R*/
char *alloc(int n)
{
	if(allocbuf + ALLOCSIZE - allocp >= n) {
		allocp += n;
		return allocp -n;
	}else{
		return NULL;
	}
}
void afree(char *p)
{
	if (p >= allocbuf && p < allocbuf + ALLOCSIZE)
		allocp = p;

}
void putc(char c)
{
	console_putc(c);
}

int puts(char *string)
{
	int cnt = 0;

	while(*string != NULL)
	{
		putc(*string++);
		cnt++;
	}

	return cnt;
}

/* this could be better */
static char *strip_zeros(char *str)
{
	char *tmp = str;
	if(str[0] != '0')
		return str;
	if(str[1] == 0)
		return str;
	while(*++tmp == '0');// && (*tmp + 2) != 0);
	return tmp;
}
static char *int_to_string(int num, int base, int size)
{
	int i;
	char *tmp = allocp;//alloc(size);
	for(i = 0; i < size; i++)
		tmp[i] = 0;
	char * ascii = {"0123456789ABCDEF"};
	if(num == 0)
	{
		tmp[0] = '0';
		tmp[1] = 0;
		return tmp;
	}
	switch(base)
	{
		case 2:
			for(i = size-1; i >=0; i--)
			{
				tmp[i] = ascii[num & 0x1];
				num >>= 1;	
			}
			break;
		case 10:
			for(i = size-1; i >=0; i--)
			{
				tmp[i] = ascii[num % 10];
				num /= 10;	
			}
			break;
		case 16:
			for(i = 7; i >= 0; i--)
			{
				tmp[i] = ascii[num & 0xF];
				num >>= 4;	
			}
			break;
		default:
			puts("invalid base given to int_to_string");
			
			break;		
	}

//	tmp[size+1] = '\0';
	return tmp;
}
int vsprintf(char *buf, const char *fmt, __gnuc_va_list args)
{
	va_list ap = args;
 	ap = ap;
	buf = buf;
	fmt = fmt;
	args = args;

	
	return -1;
}
/* based on minprintf from K&R page 156 */
void printf(char *fmt, ...)
{
	va_list ap;
 	char *p;
	char *s_val;
	char *strip;
	char c_val;
	int i_val;
	double d_val;

	va_start(ap, fmt);
	for(p = fmt; *p; p++)
	{
		if(*p != '%'){
			putc(*p);
			continue;
		}
		switch(*++p)
		{
			case 'b':
				i_val = va_arg(ap, int);
				s_val = int_to_string(i_val, 2, 32);
				strip = strip_zeros(s_val);
				puts(strip);
			//	afree(s_val);
				break;

			case 'c':
				c_val = va_arg(ap, int);
				putc(c_val);
				break;
			case 'd':
			case 'i':
				i_val = va_arg(ap, int);
				s_val = int_to_string(i_val, 10, 10);
				strip = strip_zeros(s_val);
				puts(strip);
			//	afree(s_val);
				break;
			case 'f':
				d_val = va_arg(ap,double);
				d_val = d_val;
				break;
			case 's':
				s_val = va_arg(ap, char *);
				puts(s_val);
			//	afree(s_val);
				break;
			case 'X':
				puts("0x");	
			case 'x':
				i_val = va_arg(ap, int);
				s_val = int_to_string(i_val, 16, 8);
				strip = strip_zeros(s_val);
				puts(strip);
			//	afree(s_val);
				break;
			
			default:
				putc(*p);
				break;
		}
		for(int i= 0; i < 100; i++)
			allocp[i] = 0;
	}
	
	va_end(ap);


}
