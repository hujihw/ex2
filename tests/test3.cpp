#include "../uthreads.h"
#include <iostream>
#include <fstream>

using namespace std;

void f (void)
{
	int vtq_thread_run = 1;
	while(1)
	{
		if (vtq_thread_run == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "\n-------------------" << uthread_get_total_quantums()<< "----------------------\n" << endl;
			cout << "	  f" << "  q:  " << uthread_get_tid() << endl;
			cout << "	  running = " << uthread_get_quantums(uthread_get_tid())<< endl;
			
			
			if (vtq_thread_run == 1 )
			{

				cout << "f go to sleep for 3 quantums" << endl;
				uthread_sleep(3);
				
			}

			if (vtq_thread_run == 2 )
			{

				cout << "f BLOCKED" << endl;
				uthread_block(uthread_get_tid());
				
			}
			if (vtq_thread_run == 3 )
			{
				cout << "f block the main thread :error" << endl;
				uthread_block(0);
			}
			
			
			
			if (vtq_thread_run == 7 )
			{
				
				cout << "   *** f terminate itself ****" << endl;
				uthread_terminate(uthread_get_tid());
				return;
			}
			vtq_thread_run++;
		}
	}
}

void g (void)
{
	int vtq_thread_run = 1;
	int j = 0;
	while(1)
	{
		if (vtq_thread_run == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "\n-------------------" << uthread_get_total_quantums()<< "----------------------\n" << endl;
			cout << "	  g" << "  q:  " << uthread_get_tid() << endl;
			cout << "	  running = " << uthread_get_quantums(uthread_get_tid())<< endl;
			
			if(vtq_thread_run == 1)
			{
				cout << " g resume thread in sleeping : no effect" << endl;
				uthread_resume(1);	
			}
			

			if(vtq_thread_run == 4)
			{
				cout << " f resume by g" << endl;
				uthread_resume(1);	
			}

			if (vtq_thread_run == 8)
			{
	
				j++;
				cout << "   *** g terminate itself ****" << endl;
				uthread_terminate(uthread_get_tid());
				return;
			}
			
			vtq_thread_run++;
		}
	}
}


int main(void)
{
cerr << "------------------ start test3 ------------------\n" << endl;	uthread_init(1000);
	uthread_spawn(f);
	uthread_spawn(g);
	
	int vtq_thread_run = 1;
	while(1)
	{
		if (vtq_thread_run == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "\n-------------------" << uthread_get_total_quantums()<< "----------------------\n" << endl;
			cout << "	  main" << "  q:  " << uthread_get_tid() << endl;
			cout << "	  running = " << uthread_get_quantums(uthread_get_tid())<< endl;
			if (vtq_thread_run == 1)
			{
				cout << "main thread go to sleep : error" << endl;
				uthread_sleep(0);
			}		
			
			if (vtq_thread_run == 2)
			{
				cout << "terminate a thread with id not exist:error" << endl;
				uthread_terminate(5);
			}
			
			
			if (vtq_thread_run == 10)
			{
				cout << "   *** main terminate itself ****" << endl;
				cout << "total quantums:  " << uthread_get_total_quantums() << endl;
				uthread_terminate(0);
				return 0;
			}
			vtq_thread_run++;
		}
	}
	
	
			
	
}
