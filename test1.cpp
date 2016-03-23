#include "uthreads.h"
#include <iostream>
#include <fstream>
using namespace std;

void f (void)
{
	int i = 1;
	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			  cout << "\n----------------" << uthread_get_total_quantums() << "----------------\n" << endl;
			  cout << "	f :" << "  q:  " << uthread_get_tid() << endl;
			  cout << "	running = " << uthread_get_quantums(uthread_get_tid()) << endl;
			if (i == 3)
			{
				  cout << "	f suspend by f" << endl;
				uthread_block(uthread_get_tid());
				
			}
			if (i == 6)
			{
				  cout << "	g resume by f" << endl;
				uthread_resume(3);
			}
			if (i == 8)
			{
				  cout << "	**f end**" << endl;
				uthread_terminate(uthread_get_tid());
				return;
			}
			i++;
		}
	}
}

void g (void)
{
	int i = 1;
	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			  cout << "\n----------------" << uthread_get_total_quantums() << "----------------\n" << endl;
			  cout << "	g :" << "  q:  " << uthread_get_tid() << endl;
			  cout << "	running = " << uthread_get_quantums(uthread_get_tid()) << endl;
			if(i == 5)
			{
				  cout << "	f resume by g" << endl;
				uthread_resume(1);	
			}
			if (i == 11)
			{
				  cout << "	  ** g end **" << endl;
				uthread_terminate(uthread_get_tid());
				return;
			}
			
			i++;
		}
	}
}


int main(void)
{
	cerr << "------------------ start test1 ------------------\n" << endl;
	uthread_init(100);
	uthread_spawn(f);
	uthread_spawn(g);
	int i = 1;
	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			  cout << "\n----------------" << uthread_get_total_quantums() << "----------------\n" << endl;
			  cout << "	main :" << "  q:  " << uthread_get_tid() << endl;
			  cout << "	running = " << uthread_get_quantums(uthread_get_tid()) << endl;
			if (i == 18)
			{
				  cout << "	  ******end******" << endl;
				  cout << "total quantums:  " << uthread_get_total_quantums() << endl;
				uthread_terminate(0);
				return 0;
			}
			i++;
		}
	}
	
	
			
	
}




