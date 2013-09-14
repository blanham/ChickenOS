#include <stdio.h>
#include <unistd.h>

int i = 0;

int main(int argc, char **argv)
{



    while (i < 3)
    {
        fork();
        printf("pid = %d, i = %d\n", getpid(), i);
        ++i;
    }
    return 0;




	int ret = 0;

	if(!(ret = fork()))
	{
		printf("D %i\n", ret);
	if(!(ret = fork()))
		{
			printf("C %i\n",ret);
		while(1);



		}
		printf("A %i\n", ret);

		while(1);

	}
	printf("B %i\n", ret);

	for(;;){}
	return 0;
}
