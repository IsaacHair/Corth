#include <stdio.h>
#include <stdlib.h>

main()
{
  int x = 1;
  if (1)
    x = 2, printf("some stuff\n"), printf("%d\n", x);
}
