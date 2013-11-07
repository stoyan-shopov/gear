#include "base.hxx"

 
struct str
{
	int	f1	: 2;
	int	: 2;
	int	f2	: 1;
	int	f3	: 5;
	char	f4	: 4;
	int	f5	: 3;
	long long	longint : 57;
} str;

int arr[1][2][3];

base obj1;

int base::y = 100;

int main(void)
{
class base * obj2 = new base;

	return obj1.f1() / obj2->f3() - obj2->f2() - tadd<int>(3, 3);
}

