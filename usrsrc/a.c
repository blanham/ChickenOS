#include <stdio.h>
int main(int argc, char **argv)
{
	printf("hello, world %x\n", argc);
	if(fork())
	{
		printf("parent!\n");
		while(1);
		exit(0);

	}
	printf("child\n");
	while(1);
}

