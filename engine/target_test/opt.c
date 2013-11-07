

int sub(int x, int y)
{
	x -= y;
	x <<= y * 5;
	x += 123 ^ (x | y);
	x *= y/x;
	return x - y;
}

int add_1(int x, int y)
{
	if (!x)
		return 1;
	return add_1(x--, y);
}


static int sum(int a, int b)
{
	if (a > b)
		return a;
	else
		return a + b;

}



int sum_mult(int x, int y, int z)
{
int res;
int i;

	res = sum(x, y) + sum(y, z);
	for (i = 0; i < 10; i++)
		res += sum(res, i);
	return res;
}


