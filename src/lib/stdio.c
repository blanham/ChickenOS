#include <kernel/console.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
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

char getchar()
{
	return kbd_getc();
}

char *gets(char *str)
{
	char c;
	char *tmp = str;
	while((c = getchar()) != '\n')
	{
		*tmp++ = c;
	}

	*tmp++ = '\0';
	return str;
}

int puts(char *string)
{
	int cnt = 0;
	if(string == NULL)
	{
		return puts("(null)");
	}
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
	if(str[0] == '-')
	{
		return str;
	}
	while(*++tmp == '0');// && (*tmp + 2) != 0);
	return tmp;
}

char neg[2] = {'-','0'};
char dp[256];
static char *int_to_string(int num, int base, int size)
{
	int i;
	char *tmp = allocp;//alloc(size);
	for(i = 0; i < 100; i++)
		tmp[i] = 0;
	char * ascii = {"0123456789ABCDEF"};
	memset(dp, 0, 256);	
//	if(sign)
//		tmp++;	
	/*int start = 0;
	int temp = num;
	if((num < 0) && (base == 10))
	{
	//	start = 1;
	//	size += 1;
		*tmp++ = '-';
		num = -1 * num;
	}*/
	if(num < 0 && base == 10)
	{
		strcat(dp, "-");
		num = -1 * num;
	}
	if(num == 0)
	{
		tmp[0] = '0';
		tmp[1] = 0;
		return tmp;
	}


	switch(base)
	{
		case 2:
			for(i = size-1; i >= 0; i--)
			{
				tmp[i] = ascii[num & 0x1];
				num >>= 1;	
			}
			break;
		case 10:
			for(i = size-1; i >= 0; i--)
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
//	if(temp < 0)
	//	tmp--;
//	tmp[size+1] = '\0';
	tmp = strip_zeros(tmp);
	strcat(dp, tmp);
	return dp;
}

/* based on minprintf from K&R page 156 */
void oprintf(char *fmt, ...)
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


/* based on minprintf from K&R page 156 */
int vsprintf(char *buf, const char *fmt, va_list args)
{
	va_list ap = args;
 	char *p;
	char *s_val;
	char *strip;
	char c_val;
	int i_val;
	double d_val;
	char cbuf[2] = {0,0};
	
	for(p = (char *)fmt; *p; p++)
	{
		if(*p != '%'){
			cbuf[0]  = *p;
			strcat(buf, cbuf);
			continue;
		}

		switch(*++p)
		{
			case 'b':
				i_val = va_arg(ap, int);
				s_val = int_to_string(i_val, 2, 32);
				strip = strip_zeros(s_val);
				strcat(buf,strip);
				//puts(strip);
			//	afree(s_val);
				break;

			case 'c':
				c_val = va_arg(ap, int);
				c_val = c_val;
				//putc(c_val);
				cbuf[0]  = c_val;
				strcat(buf, cbuf);

				break;
			case 'u':
			case 'd':
			case 'i':
				i_val = va_arg(ap, int);
				s_val = int_to_string(i_val, 10, 10);
				strip = strip_zeros(s_val);
				//puts(strip);
				strcat(buf,strip);
			//	afree(s_val);
				break;
			case 'f':
				d_val = va_arg(ap,double);
				d_val = d_val;
				break;
			case 's':
				s_val = va_arg(ap, char *);
				//puts(s_val);
				if(s_val != NULL)
					strcat(buf,s_val);
				else
					strcat(buf,"(null)");
			//	afree(s_val);
				break;
			case 'X':
				//puts("0x");	
				strcat(buf,"0x");
			case 'x':
				i_val = va_arg(ap, int);
				s_val = int_to_string(i_val, 16, 8);
				strip = strip_zeros(s_val);
				//puts(strip);
				strcat(buf,strip);
			//	afree(s_val);
				break;
			default:
				//console_puts("ASFZSDFSDAF");
				//putc(*p);
				break;
		}
		for(int i= 0; i < 100; i++)
			allocp[i] = 0;
	}
	strip = strip;	
	return 0;//buf - _buf;
}
extern int linux_vsprintf(char *buf, const char *fmt, va_list args);
int printf(const char *fmt, ...)
{
	va_list ap;
	char buf[512];
	int ret;
	memset(buf, 0, 512);
	va_start(ap, fmt);
	ret = linux_vsprintf(buf, fmt, ap);
	puts(buf);
	//console_puts("\n");
	va_end(ap);
	return ret;
}

int sprintf(char *buf, const char *fmt, ...)
{
	va_list ap;
	int ret;
	va_start(ap, fmt);
	ret = vsprintf(buf, fmt, ap);
//	puts(buf);
	va_end(ap);
	return ret;
}


