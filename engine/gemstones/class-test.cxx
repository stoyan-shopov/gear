

class base_0
{
	private: int x;
	protected: int get_x(void);// { return x; }
};

class base_1
{
	private: int x;
	protected: int get_x(void) { return x; }
};

class derived : public base_0, public base_1
{
	public: int get_x(void) { return base_0::get_x() + base_1::get_x(); }
};

derived derived_obj;

int fget(void)
{
	return derived_obj.get_x();
}

