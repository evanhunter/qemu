

#include <memory>
#include <cstdlib>

using namespace std;

struct blaah
{
	int a;
};

struct blaah2
{
	int a;
	blaah2() : a(2) {}
};

template<typename _Tp>
struct free_deleter
{
    void operator()(_Tp* __ptr) const
    {
    	static_assert(std::is_pod<_Tp>::value, "Only for use on Plain-old-Data types");
  		free(__ptr);
    }
};

template<typename _Tp>
using unique_pod_ptr = unique_ptr<_Tp,free_deleter<_Tp>>;

template<typename _Tp>
inline unique_pod_ptr<_Tp> make_unique_pod_cleared()
{
	static_assert(std::is_pod<_Tp>::value, "Only for use on Plain-old-Data types");
	return unique_pod_ptr<_Tp>((_Tp*)calloc(1, sizeof(_Tp)));
}




template<typename _Tp>
using void_free_function = void (*)(_Tp*);

template<typename _Tp>
struct void_function_free_deleter
{
public:
	void_free_function<_Tp> free_func;

	void_function_free_deleter( void_free_function<_Tp> free_func_in )
		: free_func( free_func_in )
	{
	}

    void operator()(_Tp* __ptr) const
    {
    	free_func(__ptr);
    }
};


template<typename _Tp>
class unique_funcfree_ptr : public unique_ptr<_Tp,void_function_free_deleter<_Tp>>
{
public:
	unique_funcfree_ptr( void_free_function<_Tp> freer_func, _Tp* __ptr )
		: unique_ptr<_Tp,void_function_free_deleter<_Tp>>( __ptr, void_function_free_deleter<_Tp>(freer_func))
	{
	}
};



struct BN
{
	int g;
};

BN* do_BN(int x, int y)
{
	return (BN*)0;
}

void BN_free(BN* num)
{
	num->g = 1;
}

int main()
{
	unique_pod_ptr<blaah> b = make_unique_pod_cleared<blaah>();
	unique_pod_ptr<int[10]>   y(make_unique_pod_cleared<int[10]>());

//	unique_pod_ptr<blaah2> f = make_unique_pod_cleared<blaah2>();

	unique_funcfree_ptr<BN>(BN_free, do_BN(1,2));

	return 0;
}





