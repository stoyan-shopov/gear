#include <queue>
#include <list>

using namespace std;

class base_class_1
{
private:
	list<int> ilist;
protected:
	queue<list<int > > qlist;
public:
	base_class_1() : ilist(0)
	{
		ilist.push_front(0xdaeba);
		qlist.push(ilist);
	}
};

