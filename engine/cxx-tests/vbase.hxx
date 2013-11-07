
class vbase
{

	protected:
		virtual int vbase_pure_firtual_func(int) = 0;
		virtual double vbase_func(double x, int c)
		{
			while (c --)
				x *= x;
			return x;
		}
	static int xxx;	
	static int yyy;	
	static int zzz;	

};
