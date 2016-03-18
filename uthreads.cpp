#include <sys/time.h>
#include <csetjmp>
#include <cstdio>
#include <signal.h>
#include <vector>
#include <errno.h>
#include <iostream>
#include "uthreads.h"

#define STACK_SIZE 4096
#define MAIN_THREAD 0

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
            "rol    $0x11,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
		"rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#endif

//////////////////////
/// Uthreads Utils ///
//////////////////////

namespace uthreads_utils {
    itimerval timer;
    sigaction sig_handler;
    enum status_t {Ready, Blocked, Sleeping, Running};
    unsigned int generalQuantaCounter = 1;
    Thread threads[MAX_THREAD_NUM];
    sigjmp_buf env[MAX_THREAD_NUM];
    std::vector<Thread> readyThreads; // todo check better option than vector
    std::vector<Thread> blockedThreads;
    std::vector<Thread> sleepingThreads; // todo remove if not needed
    Thread* runningThread;
    int quantumLength;

    // the scheduler mechanics
    void timer_handler(int sig)
        {
            // Block SIGVTALRM
            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set, SIGVTALRM);
            sigprocmask(SIG_SETMASK, &set, NULL);

            //todo handle signal

            // Round Robin alg.
            for (std::vector<Thread>::iterator it = sleepingThreads.begin();
                    it != sleepingThreads.end(); ++it) {
                // todo if sleeping over, push_back to ready
            }

            if (runningThread->getStatus() == Blocked) {

            }


            // unblock SIGVTALRM
            sigprocmask(SIG_BLOCK, &set, NULL);

            int ret_val = sigsetjmp(runningThread->env,1);

            // reset the timer
            timer.it_value.tv_usec = quantumLength;

            if (ret_val == 1) {
                return;
            }

            siglongjmp(readyThreads[0].env,1); // todo decide how to
                                               // implement the queue
        }
}

///////////////////////////////
/// Library Implementations ///
///////////////////////////////

using namespace uthreads_utils;

int uthread_init(int quantum_usecs) {

    // initialize the quantum length in micro seconds.
    quantumLength = quantum_usecs;

    // check that parameter is positive
    if (quantum_usecs <= 0) {
        std::cerr << "thread library error: parameter quantum_usecs must be positive" << std::endl;
        return -1;
    }

    // Install timer_handler as the signal handler for SIGVTALRM.
    sig_handler.sa_handler = timer_handler;
    if (sigaction(SIGVTALRM, &(sig_handler), NULL) < 0) {
        std::cerr << "system error: sigaction failed with errno: " << errno << std::endl;
        exit(1);
    }

    // Configure the timer to expire after "quantum_usecs".
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = quantum_usecs;

    // configure the timer to not execute an interval.
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer(ITIMER_VIRTUAL, &(timer), nullptr)){
        std::cerr << "system error: sigaction failed with errno: " << errno << std::endl;
        exit(1);
    }

    Thread main_thread(nullptr, MAIN_THREAD);
    threads[0] = main_thread;
    return 0;
}

////////////////////
/// class Thread ///
////////////////////

class Thread {
    using namespace uthreads_utils;

public:
    sigjmp_buf env;
    Thread(void *entryPoint, const unsigned int id);

private:
public:
    unsigned int getId() const {
        return id;
    }

    unsigned int getQuantaCounter() const {
        return quantaCounter;
    }

private:
    unsigned int id;
    status_t status;
    void *entry_point;
public:
    void setQuantaCounter(unsigned int quantaCounter) {
        Thread::quantaCounter = quantaCounter;
    }

    void setStatus (status_t new_status){
        Thread::status = new_status;
    }

    const status_t getStatus() const {
        return status;
    }

private:
    unsigned int quantaCounter;
    char stack[STACK_SIZE];
};

//////////////////////////////
/// Thread Implementations ///
//////////////////////////////

Thread::Thread(void *entryPoint, const unsigned int id) {
    Thread::id = id;
    Thread::entry_point = entryPoint;
}
