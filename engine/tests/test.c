
int var_0 = 0;
int var_1 = 1;
int var_2 = 2;
int var_3 = 3;

int arr[] = { 1, 2, 3, 4, 5,};
int * intptr = arr;
int imtx[3][2] = { {1, 2}, {3, 4}, {5, 6}};
char cmtx[2][3] = { {1, 2, 3}, {4, 5, 6},};

int intarr[3][4] = { 1,2,3,4,6,7,8, };

struct
{
	char	string[2];
	struct	point
	{
		int x, y;
	} point;
	int arr[3];

} str[3][2], * strp = str[0], mistr[2][2];

struct mistr
{
	int a;
	char c;
	char * p;
	struct
	{
		int x, y;
		struct
		{
			struct internal
			{
				struct {
					int x;
					int y;
					struct asd
					{
						int c;
					} casd;
				}x;
				int child;
			} x;
		}z;
	}
	point;
	int	f;

} mistr1;

struct str_1 { int x;};
union str_2 {void * f; };
enum str_3 { INV = 0 };

int foo(int x)
{
	volatile struct foo {int x; } s;
	s.x = x + 1;
	return s.x;
}

float bar(float x)
{
	volatile struct foo {float x; } s;
	s.x = x + 1;
	return s.x;
}


void(*fptr)(void);

void(*fptr1(void))(void);

int main(void)
{
char carr[2], (*cptr)[2];

	(&carr)[1][2] = 1;
	(&carr);
	cptr = &carr;
	return 0;
}

