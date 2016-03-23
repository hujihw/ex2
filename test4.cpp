#include "uthreads.h"
#include <iostream>
#include <fstream>

using namespace std;
void f(void);
void g();
void h();
void t();
void v();
void w();
void q();

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
			if ( i == 1)
			{
				cout << "	  h create by f"  << endl;
				uthread_spawn(h);
			}
			if ( i == 2)
			{
				cout << "h resumed by f"  << endl;
				uthread_resume(3);
			}
			if (i == 3)
			{
				cout << "	  main blocked by f" << endl;
				uthread_block(0);
				
			}
			
			if (i == 4)
			{
				cout << "	 g wakeup after =" << uthread_get_time_until_wakeup(2) <<endl;
			}
			if (i == 5)
			{
				cout << "	  h resumed by f" << endl;
				uthread_resume(3);
			}
			if (i == 6)
			{
				cout << "	  f go to sleep 6 quantum" << endl;
				uthread_sleep(6);
			}
			if (i == 7)
			{	
				cout << "	  f terminated by  f" << endl;
				cout << "	  total running q: " << uthread_get_quantums(1)<< endl;
				cout << "	  ** f end **" << endl;
				uthread_terminate(1);
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
				cout << "	g blocked by g " << endl;
				uthread_block(2);
			}
			if (i == 2)
			{
				cout << "	  f resumed by g" << endl;
				uthread_resume(1);
			}
			if (i == 3)
			{
				cout << "	  g sleep 10 quantums" << endl;
				uthread_sleep(10);
			}
			if ( i== 4)
			{
				cout << "	  f wakeup after = " << uthread_get_time_until_wakeup(1) <<endl;
				uthread_get_time_until_wakeup(1);
			}
			if(i == 5)
			{
				cout << "	  t create by g" << endl;
				uthread_spawn(t);	
			}
			if (i == 6)
			{
				cout << "	  t resumed by g" << endl;
				uthread_resume(3);
				
			}
			i++;
		}
	}
}

void h()
{
	int i = 1;
	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "\n-------------------" << uthread_get_total_quantums()<< "----------------------\n" << endl;
			cout << "	  h" << "  q:  " << uthread_get_tid() << endl;
			cout << "	  running = " << uthread_get_quantums(uthread_get_tid())<< endl;
			if (i == 1)
			{
				cout << "	  try to block f" << endl;
				uthread_block(1);
			}
			if (i == 2)
			{
				cout << "	  g wakeup after =" << uthread_get_time_until_wakeup(2) << endl;
			}
			if (i == 3)
			{
				cout << "	  g wakeup after =" << uthread_get_time_until_wakeup(1) << endl;
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
				cout << "	  g blocked by t" << endl;
				uthread_block(2);
			}
			if ( i == 2)
			{
				cout << "	  g terminate by t" << endl;
				cout << "	  total running q: " << uthread_get_quantums(2)<< endl;
				cout << "	  ** g end **" << endl;
				uthread_terminate(2);
			}
			if ( i == 3)
			{
				cout << "	  v created by t" << endl;
				uthread_spawn(v);
			}
			if (i == 4)
			{
				cout << "	  try to sleep" << endl;
				uthread_sleep(-5);
			}
			if (i == 5)
			{
				cout << "	  w blocked by t" << endl;
				cout << "	  t go to sleep 2 quantum" << endl;
				uthread_block(1);
				uthread_sleep(2);
			}
			if (i == 6)
			{
				cout << "	  w resumed by t" << endl;
				uthread_resume(1);
			}
			if (i == 7)
			{
				cout << "	  t terminated by t" << endl;
				cout << "	  total running q: " << uthread_get_quantums(3)<< endl;
				cout <<"	  ** t end **" << endl;
				uthread_terminate(3);
				return;
			}
			i++;
		}
	}
}
void w()
{
	int i = 1;
	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "\n-------------------" << uthread_get_total_quantums()<< "----------------------\n" << endl;
			cout << "	  w" << "  q:  " << uthread_get_tid() << endl;
			cout << "	  running = " << uthread_get_quantums(uthread_get_tid())<< endl;
			if (i == 1)
			{
				cout << "	  v blocked by w" << endl;
				uthread_block(2);
			}
			if ( i == 2)
			{
				cout << "	  v resumed by w" << endl;
				uthread_resume(2);
			}
			if ( i == 3)
			{
				cout << "	  v blocked by w" << endl;
				uthread_block(2);
			}
			if (i == 4)
			{
				cout << "	  q created by w" << endl;
				uthread_spawn(q);
			}
			i++;
		}
	}
}

void v()
{
	int i = 1;
	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "\n-------------------" << uthread_get_total_quantums()<< "----------------------\n" << endl;
			cout << "	  v" << "  q:  " << uthread_get_tid() << endl;
			cout << "	  running = " << uthread_get_quantums(uthread_get_tid())<< endl;
			if (i == 1)
			{
				cout << "	  t blocked by v" << endl;
				uthread_block(3);
			}
			
			i++;
		}
	}
}

void q()
{
	int i = 1;
	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "\n-------------------" << uthread_get_total_quantums()<< "----------------------\n" << endl;
			cout << "	  q" << "  q:  " << uthread_get_tid() << endl;
			cout << "	  running = " << uthread_get_quantums(uthread_get_tid())<< endl;
			if (i == 1)
			{
				cout << "	  w terminated by q" << endl;
				cout << "	  total running q: " << uthread_get_quantums(1)<< endl;
				cout << "	  ** w end **" << endl;
				uthread_terminate(1);
			}
			if ( i == 2)
			{
				cout << "	  q go to sleeo for 1 quantum" << endl;
				uthread_sleep(1);
			}
			if (i == 3)
			{
				cout << "	  q blocked by q" << endl;
				uthread_block(2);
			}
			i++;
		}
	}
}
int main(void)
{
	cerr << "------------------ start test4 ------------------\n" << endl;
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
			if(i == 2)
			{
				cout << "	f blocked by main" << endl;
				uthread_block(1);
			}
			if (i == 3)
			{
				cout << "	h blocked by main" << endl;
				uthread_block(3);
			}
			if (i == 4)
			{
				cout << "	g resumed by main" << endl;
				uthread_resume(2);
			}
			if (i == 5)
			{
				cout << "	main thread try to sleep" << endl;
				uthread_sleep(1);
			}
			if (i == 7)
			{
				cout << "	h blocked by main" << endl;
				uthread_block(3);
			}
			if ( i == 8)
			{
				cout << "	  try to resumed f by main thread" << endl;
				uthread_resume(1);
			}
			if (i == 9)
			{
				cout << "	  try to block g" << endl;
				uthread_block(2);
			}
			if(i == 10)
			{
				cout << "	  h terminated by main" << endl;
				cout << "	  total running q: " << uthread_get_quantums(3)<< endl;
				cout << "	  ** h end **" << endl;
				uthread_terminate(3);
			}
			if (i == 12)
			{
				cout << "	try to block f" << endl;
				uthread_block(1);
			}
			if (i == 13)
			{
				cout << "	t blocked by main" << endl;
				uthread_block(3);
			}
			if ( i == 14)
			{
				cout << "	  g resumed by main" << endl;
				uthread_resume(2);
			}
			if (i == 16)
			{
				cout << "  w created by main" << endl;
				uthread_spawn(w);
			}
			if(i == 17)
			{
				cout << "	main sleep by main " << endl;
				uthread_sleep(1);
			}
			if (i == 18)
			{
				cout << "	main blocked by main" << endl;
				uthread_block(0);
			}
			
			if (i == 21)
			{
				cout << "	v terminated by main" << endl;
				cout << "	  total running q: " << uthread_get_quantums(2)<< endl;
				cout << "	  ** v end **" << endl;
				uthread_terminate(2);
			}
			if (i == 26)
			{
				cout << "	  total running q: " << uthread_get_quantums(2)<< endl;
				cout << "	    ** q end **" << endl;
				cout << "	  total running main: " << uthread_get_quantums(uthread_get_tid()) << endl;
				cout << "	    ** main end **" << endl;
				cout << "total quantums:  " << uthread_get_total_quantums() << endl;
				cout << "	  *** end ****" << endl;
				uthread_terminate(0);
				return 0;
			}
			i++;
		}
	}	
}


