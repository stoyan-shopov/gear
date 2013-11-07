
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

int main_old(void)
{
char carr[2], (*cptr)[2];

	(&carr)[1][2] = 1;
	(&carr);
	cptr = &carr;
	return 0;
}


