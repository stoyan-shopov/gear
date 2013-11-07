int func1(void);
int func2(void) { return 0; }

int (*func_ptr)(void) = func2;

int main(void)
{
        return func1();
}

int fib(int n)
{
int res;
        if (n < 2)
                res = n;
        else
                res = fib(n - 1) + fib(n - 2);
        return res;
}

static int inline inline_test(int x)
{
        return x++;
}

int func1(void)
{
        return func_ptr() + inline_test((int)func1);
}
