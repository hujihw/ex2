#include <sys/time.h>
#include <csetjmp>
#include <cstdio>
#include <signal.h>
#include <vector>
#include <errno.h>
#include <iostream>
#include <list>
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
    std::vector<Thread*> readyThreads; // todo check better option than vector
    std::vector<Thread*> blockedThreads;
    std::vector<Thread*> sleepingThreads; // todo remove if not needed
    Thread* runningThread;
    Thread* terminateThread = nullptr;
    int quantumLength;

    void wakeupSleepingThreads() {
        for (int i = 0; i < sleepingThreads.size(); ++i) {
            (*sleepingThreads[i]).decreaseSleepingCountdown();
            if ((*sleepingThreads[i]).getSleepingCountdown() == 0) {
                (*sleepingThreads[i]).setStatus(Ready);
                readyThreads.push_back(sleepingThreads[i]);
                sleepingThreads.erase(sleepingThreads.begin() + i);
            }
        }
    }

    // the scheduler mechanics
    void timer_handler(int sig)
    {
        // Block SIGVTALRM
        signal(SIGVTALRM, SIG_IGN);
//            sigset_t set; // todo what pending does
//            sigemptyset(&set);
//            sigaddset(&set, SIGVTALRM);
//            sigprocmask(SIG_SETMASK, &set, NULL);

        generalQuantaCounter += 1;
        runningThread->quantaCounterUp();

        ////////////////////////
        /// Round Robin alg. ///
        ////////////////////////

        if (terminateThread != nullptr){
            // todo terminate the thread
        }

        // add the threads that finished sleeping to ready vector
        wakeupSleepingThreads();

        // if the running thread has blocked itself, add it to blocked vector
        switch (runningThread->getStatus()) {
            case Running:
                runningThread->setStatus(Ready);
                readyThreads.insert(readyThreads.begin(), runningThread);
                break;

            case Blocked:
                blockedThreads.insert(blockedThreads.begin(), runningThread);
                break;

            case Sleeping:
                sleepingThreads.insert(sleepingThreads.begin(), runningThread);
                break;

            case Ready:
                std::cerr << "Error: Running thread is in ready Status while "
                                 "running!" << std::endl; // todo remove
                break;
        }

        // first thread in ready vector become running
        runningThread = readyThreads.back();
        readyThreads.pop_back();

        // unblock SIGVTALRM
        signal(SIGVTALRM, SIG_DFL);

        int ret_val = sigsetjmp(runningThread->env, 1);

        // reset the timer
        timer.it_value.tv_usec = quantumLength;

        if (ret_val == 1) {
            return;
        }

        siglongjmp((*readyThreads[0]).env, 1);
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
    sig_handler.sa_flags = 0;
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
    void quantaCounterUp();
    void setStatus(status_t new_status);
    const status_t getStatus() const;
    unsigned int getId() const;
    unsigned int getQuantaCounter() const;
    int getSleepingCountdown() const;
    void setSleepingCountdown(int sleepingCountdown);
    void decreaseSleepingCountdown();

private:
    int sleepingCountdown;
    unsigned int id;
    status_t status;
    void *entry_point;
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

unsigned int Thread::getQuantaCounter() const {
    return quantaCounter;
}

unsigned int Thread::getId() const {
    return id;
}

const status_t Thread::getStatus() const {
    return status;
}

void Thread::setStatus(status_t new_status) {
    Thread::status = new_status;
}

void Thread::quantaCounterUp() {
    Thread::quantaCounter += 1;
}

void Thread::decreaseSleepingCountdown() {
    if (sleepingCountdown > 0) {
        sleepingCountdown--;
    }
}

void Thread::setSleepingCountdown(int sleepingCountdown) {
    Thread::sleepingCountdown = sleepingCountdown;
}

int Thread::getSleepingCountdown() const {
    return sleepingCountdown;
}


/////////////////////////////////////////
/// Library Functions Implementations ///
/////////////////////////////////////////

int uthread_block(int tid){
    return 0;
}
