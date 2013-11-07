class base;

class base * base_ptr;

class vbase
{
	friend char foo(void);
	volatile int foo(void) {return 5;}
	protected:	
	static int xxx;	
};

//int vbase::xxx = 4;

char foo(void)
{
	vbase p;
	return p.foo();
}

