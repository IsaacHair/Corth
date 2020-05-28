#include <stdlib.h>
#include <stdio.h>

void main() {
	int ddf = 1;
	int df = 0;
	int f = 0;
	for (int t = 0; t < 100; t++) {
		for (int temp = 0; temp < f; temp++)
			printf(" ");
		printf(".\n");
		if (df > 4 || df < -4)
			ddf = -ddf;
		df += ddf;
		f += df;
	}
}
