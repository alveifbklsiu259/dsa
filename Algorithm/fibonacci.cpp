int fib(int idx) {
  if (idx == 0)
    return 0;

  if (idx == 1)
    return 1;
  return fib(idx - 1) + fib(idx - 2);
}