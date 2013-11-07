#include "base.hxx"

struct xstruct
{
	public:
	static int x;
	static int y;
} x1;


using namespace nm_space;

int ::nm_class::nm_xxx	= 123;
int nm_space::nm_class::nm_xxx	= 456 + x1.x;

int vbase::xxx = 100;

int base::f1(void)
{
	return x + 1;
}

