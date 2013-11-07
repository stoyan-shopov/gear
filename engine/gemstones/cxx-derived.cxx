#include "cxx-base.hxx"

class derived_class : public base_class_1
{
public:
	void insert_int(int i);
};

void derived_class::insert_int(int i)
{
	qlist.push(*(new list<int>(i)));
}


int main(void)
{
class derived_class der;

	der.insert_int(0xdaeba);
	return 0;
}
