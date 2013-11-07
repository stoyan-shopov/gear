

volatile int x;


int (*(***foo(void))(int (*)(void)))(void *, int)
{
}

void (*bar(void))(void)
{
}


int fhack(int (*fptr_1)(int ccc))
{
	return x + x - 10 * fptr_1(x++ + ++x);
}

static inline int f0(void)
{
	if (x == 0)
	{
__label__ failure;
	volatile int * xxx = &x;
	inline int fff(int i)
	{
		if (!i)
			goto failure;
		return x + i;
	}
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
	volatile inline int cascade_0(int i)
	{
		int scope_1 = i++;

		volatile inline int cascade(int i)
		{
			int scope_2 = i++ + scope_1 + scope_0;
			volatile inline int cascade(int i)
			{
				int scope_3 = scope_0 = i++;
				return x * i;
			}
			return cascade(x) + scope_0;
		}
		return cascade(i * 10);

	}
	return cascade_0(i);
}

static inline int sub0(int x)
{
volatile int var_arr[x];

	if (x)
	{
		volatile struct xtype { int i; } * xxx = 0;
		return cascade(xxx->i);
	}
	else if (x == 1)
	{
		volatile int * zzz = (volatile int *) 100;
		volatile struct ztype { int i; } * xxx = 0;
		return *zzz + xxx->i;
	}
	return var_arr[x / 2];
}

int (* get_arr(void))[5] {}
//void *fptr_arr[5](void);	// array of functions
int (* (*fptr_arr[5])(void))[3];
//int * get_arr(void) {}

int (*iptr)[5];

int (*(*((*(*(*(* fptr_2[36][56])[67])(void))[12])(int, char, int * (*)(char))))(int))[23] = { [5] = 10, [3] = 0xaa55, };
int int_arr[5] = { 0, 1, 2, 3, 4, }, * iparr[5] = { [0] = int_arr, [1] int_arr + 1, [2] = int_arr + 2,
	[3] = int_arr + 3, [4] = int_arr + 4, }, iarr1[3][4], iarr2[2][1][2], iarr3[3][4][5][6][7][8], iarr4[1], iarr5[2][1], iarr6[1][1];

int main(void)
{
	return f2() + sub0(x) + main_();
}

struct
{
	char	string[2];
	struct sstr
	{
		struct sstr * next;
		struct { int a; int b; } * anon;
		struct {int a, b, cl; } anon1;
	}
	ttt;
	struct	point
	{
		int x, y;
	} point;
	int arr[3];

} str[3][2], * strp = str[0], mistr[2][2], * strarr[] = 
{
	mistr[2], mistr[2] + 1, mistr[2],
};
