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
	int count, length;

	while(*tmp != '\0')
		tmp++;
	length = tmp - str;
	

	tmp = str;
	while(*tmp == '0')
		tmp++;
	count = tmp -str;
	
	//char *new = alloc(length - count);
	
	int i;
	for(i = 0; i < length-count; i++)
	{
		str[i] = str[i + count];
	}
	for(;i < length; i++)
		str[i] = NULL;
	//afree(str);	
	return str;	
}
static char *int_to_string(int num, int base, int size)
{
	int i;
	char *tmp = alloc(size);
	char * ascii = {"0123456789ABCDEF"};
	
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
			for(i = 9; i >=0; i--)
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
				s_val = strip_zeros(s_val);
				puts(s_val);
				afree(s_val);
				break;

			case 'c':
				c_val = va_arg(ap, int);
				putc(c_val);
				break;
			case 'd':
			case 'i':
				i_val = va_arg(ap, int);
				s_val = int_to_string(i_val, 10, 10);
				s_val = strip_zeros(s_val);
				puts(s_val);
				afree(s_val);
				break;
			case 'f':
				d_val = va_arg(ap,double);
				d_val = d_val;
				break;
			case 's':
				s_val = va_arg(ap, char *);
				puts(s_val);
				afree(s_val);
				break;	
			case 'x':
				i_val = va_arg(ap, int);
				s_val = int_to_string(i_val, 16, 8);
				s_val = strip_zeros(s_val);
				puts(s_val);
				afree(s_val);
				break;

			default:
				putc(*p);
				break;
		}
	}
	
	va_end(ap);


}
