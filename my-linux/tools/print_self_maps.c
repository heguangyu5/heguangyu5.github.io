#include <stdio.h>
#include <string.h>

int main(void)
{
	FILE *fp = fopen("/proc/self/maps", "r");

	char buf[512];
	memset(buf, 0, 512);
	while (fgets(buf, 512, fp)) {
		printf("%s", buf);
	}
	fclose(fp);

	return 0;
}
