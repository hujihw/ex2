
#include "uthreads.h"
#include <iostream>
#include <fstream>

using namespace std;

void f (void)
{
	while(1);
}

int main(void)
{
	uthread_init(1000);
	for (int i = 0; i < 101; i++)
		cout << uthread_spawn(f) << endl;

	uthread_terminate(5);

	cout << uthread_spawn(f) << endl;
	cout << uthread_spawn(f) << endl;

	uthread_terminate(15);
	uthread_terminate(25);
	uthread_terminate(35);
	uthread_terminate(45);
	uthread_terminate(55);
	uthread_terminate(65);
	uthread_terminate(75);
	uthread_terminate(85);

	cout << uthread_spawn(f) << endl;
	cout << uthread_spawn(f) << endl;
	cout << uthread_spawn(f) << endl;
	cout << uthread_spawn(f) << endl;
	cout << uthread_spawn(f) << endl;
	cout << uthread_spawn(f) << endl;
	cout << uthread_spawn(f) << endl;
	cout << uthread_spawn(f) << endl;
	cout << uthread_spawn(f) << endl;


	uthread_terminate(0);
	return 0;
}


