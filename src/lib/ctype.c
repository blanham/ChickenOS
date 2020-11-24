#include <ctype.h>

int isprint(char c)
{
	if(('a' <= c) && (c <= 'z'))
	return 1;
	if(c == '/')
		return 1;
	if(c == '-')
		return 1;
	return 0;

}

int isdigit(int c)
{
	if(('0' <= c) && (c <= '9'))
		return 1;
	return 0;
}

int isxdigit(int c)
{
	if(isdigit(c) == 1)
		return 1;

	if(('a' <= c) && (c <= 'f'))
		return 1;


	if(('A' <= c) && (c <= 'F'))
		return 1;

	return 0;
}

int isalpha(int c)
{
	if(('A' <= c) && (c <= 'Z'))
		return 1;
	if(('a' <= c) && (c <= 'z'))
		return 1;

	return 0;
}

int isalnum(int c)
{
	if(isalpha(c))
		return 1;
	if(isdigit(c))
		return 1;
	return 0;
}

int islower(int c)
{
	if(('a' <= c) && (c <= 'z'))
		return 1;
	return 0;
}

int isspace(int c)
{
	return c == ' ' || (unsigned)c-'\t' < 5;
}

int toupper(int c)
{
	int dif = 'A' - 'a';
	if(c >= 'A')
		return c;

	else
		return c + dif;

}