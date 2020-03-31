#define print(x) {				\
    1;						\
    if (x)					\
      other();					\
  }
#define other() print()

main()
{
  print(0);
}
