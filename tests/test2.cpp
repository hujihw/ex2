#include "../uthreads.h"
#include <iostream>
#include <fstream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
using namespace std;
void func()
{
	int i = 1;
	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "\n-------------------" << uthread_get_total_quantums()<< "----------------------\n" << endl;
			cout << "	  func" << "  q:  " << uthread_get_tid() << endl;
			cout << "	  running = " << uthread_get_quantums(uthread_get_tid())<< endl;
			if (i == 1)
			{
				cout << "	  func terminate t" << endl;
				cout << "	  total running for t: " << uthread_get_quantums(1) << endl;
				cout << "	  ** t end **" << endl;
				uthread_terminate(1);
			}
			if (i == 2)
			{
				cout << "	  main block by func" << endl;
				
				uthread_block(0);
			}
			i++;
		}
	}
}
void t(void)
{
	int i = 1;
	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "\n-------------------" << uthread_get_total_quantums()<< "----------------------\n" << endl;
			cout << "	  t" << "  q:  " << uthread_get_tid() << endl;
			cout << "	  running = " << uthread_get_quantums(uthread_get_tid())<< endl;
			if (i == 1)
			{
				cout << "	  thread with id = " << uthread_get_tid() << endl;
			}
			if ( i == 2)
			{
				cout << "	  func create by t" << endl;
				uthread_spawn(func);
			}
			i++;
		}
	}
}
void f (void)
{
	int i = 1;
	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "\n-------------------" << uthread_get_total_quantums()<< "---------------------\n" << endl;
			cout << "	  f" << "  q:  " << uthread_get_tid() << endl;
			cout << "	  running = " << uthread_get_quantums(uthread_get_tid())<< endl;
			if ( i == 2)
			{
				int j = 3+3;
				cout << "the result of calculate 3+3="  << j << endl;
			}
			if (i == 3)
			{
				cout << "	  main blocked by f" << endl;
				uthread_block(0);
				
			}
			if (i == 5)
			{
				cout << "	  g resume by f" << endl;
				uthread_resume(2);
			}
			if (i == 6)
			{
				cout << "	  g resume by f" << endl;
				uthread_resume(2);
			}
			if (i == 7)
			{
				cout << "	  f go to sleep 5 quantum" << endl;
				uthread_sleep(5);
			}
			if (i == 8)
			{
				
				cout << "	  total running f: " << uthread_get_quantums(uthread_get_tid()) << endl;
				cout << "	  **f end **" << endl;
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
			cout << "\n-------------------" << uthread_get_total_quantums()<< "---------------------\n" << endl;
			cout << "	  g" << "  q:  " << uthread_get_tid() << endl;
			cout << "	  running = " << uthread_get_quantums(uthread_get_tid()) << endl;
			if ( i == 1)
			{
				cout << "	number of running is : " << uthread_get_quantums(uthread_get_tid()) << endl;
			}
			if (i == 3)
			{
				cout << "	  g try to sleep" << endl;
				uthread_sleep(-1);
			}
			if ( i== 4)
			{
				cout << "	  f resume by g" << endl;
				uthread_resume(1);
			}
			if(i == 5)
			{
				cout << "	  g go to sleep" << endl;
				uthread_sleep(1);	
			}
			if (i == 6)
			{
				cout << "	  g blocked by g" << endl;
				uthread_block(uthread_get_tid());
				
			}
			if (i == 9)
			{
				cout << "	  g sleep by g" << endl;
				uthread_sleep(1);
			}
			if (i == 10)
			{
				cout << "	  t create by g" << endl;
				uthread_spawn(t);
			}
			if ( i == 11)
			{
				
				cout << "	  total running g: " << uthread_get_quantums(uthread_get_tid())<< endl;
				cout << "	  **g end**" << endl;
				uthread_terminate(uthread_get_tid());
				return;
			}
			i++;
		}
	}
}


int main(void)
{
	cerr << "------------------ start test2 ------------------\n" << endl;
	uthread_init(100);
	
	
	int i = 1;
	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "\n-------------------" << uthread_get_total_quantums()<< "---------------------\n" << endl;
			cout << "	  main" << "  q:  " << uthread_get_tid() << endl;
			cout << "	  running = " << uthread_get_quantums(uthread_get_tid()) << endl;
			if (i == 1)
			{
				uthread_spawn(f);
				cout << "  created thread with func : f" << endl; 
				uthread_spawn(g);
				cout << "  created thread with func : g" << endl;
			}
			if(i == 3)
			{
				cout << "	main sleep by main " << endl;
				uthread_sleep(1);
			}
			if (i == 4)
			{
				cout << "	f blocked by main" << endl;
				uthread_block(1);
			}
			if (i == 5)
			{
				cout << "	main resume by main" << endl;
				uthread_resume(uthread_get_tid());
			}
			if (i == 6)
			{
				if (i % 2 == 0)
				{
					cout << "	  "<< i << ", is even!" << endl;
				}
			}
			if ( i == 14)
			{
				cout << "	  t blocked by main" << endl;
				uthread_block(1);
			}
			if ( i == 17)
			{
				cout << "	  t resume by main" << endl;
				uthread_resume(1);
			}
			if (i == 20)
			{
				cout << "	  total running func: " << uthread_get_quantums(2)<< endl;
				cout << "	    **func end**" << endl;
				cout << "	  total running main: " << uthread_get_quantums(uthread_get_tid()) << endl;
				cout << "	    **main end**" << endl;
				cout << "total quantums:  " << uthread_get_total_quantums() << endl;
				cout << "	  *** end ****" << endl;
				uthread_terminate(0);
				return 0;
			}
			i++;
		}
	}	
}

#pragma clang diagnostic pop