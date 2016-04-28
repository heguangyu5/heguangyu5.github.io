#include <stdio.h>
#include <unistd.h>
#include <string.h>

extern char *environ[];

int main(int argc, char *argv[])
{
	int i;
	for (i = 0; i < argc; i++) {
		printf("argv[%d] = %s\n", i, argv[i]);
	}
	
	i = 0;
	while (environ[i])	{
		printf("envp[%d] = %s\n", i, environ[i]);
		i++;
	}

	#define LEN 1024
	char buf[LEN];
	memset(buf, 0, LEN);
	if (read(3, buf, LEN - 1) == -1) {
		perror("read 3");
		return 1;
	}

	printf("continue-read print:\n%s", buf);
	return 0;
}
