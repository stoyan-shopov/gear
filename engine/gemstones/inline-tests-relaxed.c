
#define volatile

volatile int x;


int fhack(int (*fptr)(int ccc))
{
	return x + x - 10 * fptr(x++ + ++x);
}

	volatile int * xxx = &x;
	inline int fff(int i)
	{
failure:
		if (!i)
			goto failure;
		return x + i;
	}

static inline int f0(void)
{
	if (x == 0)
	{
	return fff(*xxx) * fff(10 + xxx[1]) / fff(x - 1) - fhack(fff);
failure:
	return 10;
	}
	else
	{
		volatile int * yyy = &x;
		if (!x && *yyy)
			return *yyy;
		else
			return 100;
	}
}

static inline int f1(void)
{
	return f0();
}


static inline int f2(void)
{
	return f1();
}

volatile inline int cascade(int i)
{
	int scope_0 = i++;
	return scope_0 + i;
}

static inline int sub0(int x)
{
	if (x)
	{
		volatile struct xtype { int i; } * xxx = 0;
		return cascade(xxx->i);
	}
	else
	{
		volatile int * zzz = (volatile int *) 100;
		volatile struct ztype { int i; } * xxx = 0;
		return *zzz + xxx->i;
	}
}

int main(void)
{
	return f2() + sub0(x);
}

