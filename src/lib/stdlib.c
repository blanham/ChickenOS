#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>

div_t div(int numerator, int denominator)
{
	return (div_t){ numerator/denominator, numerator%denominator};
}

// FIXME: this doesn't support different bases
long int strtol(const char* str, char **end, int base)
{
	(void)base;
	long int ret =0;
	int i, sign;
	for(i = 0; isspace(str[i]); i++)
			;
	sign = (str[i] == '-' ? -1 : 1);
	if(str[i] == '-' || str[i] == '+')
		i++;

	for(ret = 0; isdigit(str[i]); i++)
	{
		ret = 10 * ret + (str[i] - '0');
	}
	if(end != NULL)
		*end = (char *)&str[i];

	return ret * sign;
}