#include <stdio.h>
#include <string.h>
#include <unistd.h>
//#include <sys/time.h>
int main(int argc, char** argv)
{

	printf("argc \n");
	
//	printf("argv %x\n",argv);
//	int i;
//	for(i = 0; i < argc; i++)
//		printf("%s\n",argv[i]);
	int pid = 0;
	pid = fork();
	if(pid == 0)
	{

		printf("pid %i\n",pid);
		while(1);
	}
		printf("pid %i\n",pid);
//	struct timeval tv;
//	char fuck[255];
//	char *f;
//	while(1){
//	puts("g");
//	f = malloc(1024*1024*10);
//	gettimeofday(&tv, NULL);
//	int tit = tv.tv_sec;
//	printf("hello, world\n");
//}
	while(1);
	return 0;
}
