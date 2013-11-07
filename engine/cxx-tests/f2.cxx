#include "base.hxx"

struct xstruct
{
	public:
	static int x;
} x2;

int xstruct::x;

int nm_space::nm_class::nm_yyy	= 345 + x2.x;

int vbase::yyy = 200;

int base::f2(void)
{
	return x += 2;
}

