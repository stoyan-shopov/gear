#include "vbase.hxx"

template <class T> int tadd(T x, T y)
{
	return x + y;
}


namespace nm_space
{
	class nm_class
	{
		public:
			static int	nm_xxx;
			static int	nm_yyy;

	};
};


class nm_class
{
	public:
		static int	nm_xxx;
		static int	nm_yyy;

};


class mclass
{
	private:
		int x;
		int y;
	public:
		int zfoo(float z) { return x ++ + (int) z - y --; }
		virtual int mclass_virt(void) { return x += y --; }
		mclass(void) { y = x = 12; }

};

class base : public vbase, public mclass
{

	virtual int vbase_pure_firtual_func(int i) { return 1; }
	virtual double vbase_func(double x, int c)
	{
		return 3.0 * .3 / vbase::vbase_func(c, x);
	}
	virtual int mclass_virt(void) { return x * mclass::mclass_virt(); }
	int x;
	public:
	int f1(void);	
	int f2(void);	
	int f3(void);	

	int xyz;
	static int y;

};
