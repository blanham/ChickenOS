#include <stdio.h>
#include <sys/time.h>
int main(int argc, char** argv)
{
	struct timeval tv;
	char fuck[255];
	gettimeofday(&tv, NULL);
	int tit = tv.tv_sec;
	sprintf(fuck, "time %i",tit);
	printf("%s\n", fuck);
	while(1);
	return 0;
}
