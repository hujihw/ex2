#include <sys/time.h>
#include <csetjmp>
#include <cstdio>
#include <signal.h>
#include <vector>
#include <errno.h>
#include <iostream>
#include <list>
#include "uthreads.h"

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

enum status_t {Ready, Blocked, Sleeping, Running}; // todo move other
                                                   // declerations here?

////////////////////
/// class Thread ///
////////////////////

class Thread {
//    using namespace uthreads_utils;

public:
    sigjmp_buf env;
    Thread(void *entryPoint, const unsigned int id);
    ~Thread();
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
    unsigned int quantaCounter = 1;
    char *stack;
};

//////////////////////
/// Uthreads Utils ///
//////////////////////

itimerval timer;
struct sigaction sig_handler;
unsigned int generalQuantaCounter = 1;
Thread *threads[MAX_THREAD_NUM];
//    sigjmp_buf env[MAX_THREAD_NUM]; todo remove
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
    // ignore SIGVTALRM

    struct sigaction sa_ign;
    sa_ign.sa_handler = SIG_IGN;
    sigaction(SIGVTALRM, &sa_ign, nullptr);
//            sigset_t set; // todo what pending does
//            sigemptyset(&set);
//            sigaddset(&set, SIGVTALRM);
//            sigprocmask(SIG_SETMASK, &set, NULL);
    //        signal(SIGVTALRM, SIG_IGN);

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


    if (runningThread->getStatus() == Running){
        runningThread->setStatus(Ready);
        readyThreads.insert(readyThreads.begin(), runningThread);
    }


//        // if the running thread has blocked itself, add it to blocked vector
//        switch (runningThread->getStatus()) {
//            case Running:
//
//                break;
//
//            case Blocked:
//                blockedThreads.insert(blockedThreads.begin(), runningThread);
//                break;
//
//            case Sleeping:
//                sleepingThreads.insert(sleepingThreads.begin(), runningThread);
//                break;
//
//            case Ready:
//                std::cerr << "Error: Running thread is in ready Status while "
//                                 "running!" << std::endl; // todo remove
//                break;
//        }

    // first thread in ready vector become running
    runningThread = readyThreads.back();
    runningThread->setStatus(Ready);
    readyThreads.pop_back();

    // unblock SIGVTALRM
    sa_ign.sa_handler = SIG_DFL;
    sigaction(SIGVTALRM, &sa_ign, nullptr);
//        signal(SIGVTALRM, SIG_DFL);

    int ret_val = sigsetjmp(runningThread->env, 1);

    // reset the timer
    timer.it_value.tv_usec = quantumLength;

    if (ret_val == 1) {
        return;
    }

    siglongjmp((*readyThreads[0]).env, 1);
}

namespace uthreads_utils { // todo remove

}


//////////////////////////////
/// Thread Implementations ///
//////////////////////////////

Thread::Thread(void *entryPoint, const unsigned int id): id(id), entry_point
        (entryPoint), status(Ready) {
    stack = new char[STACK_SIZE];
}

Thread::~Thread() {
    delete[] stack;
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

using namespace uthreads_utils;

// helper function that checks that the id of the thread exists and is not the
// main thread
int checkTidLegallity(int tid){
    if (tid == MAIN_THREAD){
        std::cerr << "thread library error: can't block or resume "
                "the main thread" << std::endl;
        return -1;
    }

    if (threads[tid] == nullptr || tid > MAX_THREAD_NUM - 1){
        std::cerr << "thread library error: parameter tid must be refer to an "
                "id of an existing thread" << std::endl;
        return -1;
    }
    return 0;
}

int uthread_init(int quantum_usecs) {

    // initialize the quantum length in micro seconds.
    quantumLength = quantum_usecs;

    // check that parameter is positive
    if (quantum_usecs <= 0) {
        std::cerr << "thread library error: parameter quantum_usecs must be "
                "positive" << std::endl;
        return -1;
    }

    // Install timer_handler as the signal handler for SIGVTALRM.
    sig_handler.sa_handler = timer_handler;
    sig_handler.sa_flags = 0;
    if (sigaction(SIGVTALRM, &(sig_handler), NULL) < 0) {
        std::cerr << "system error: sigaction failed with errno: " <<
        errno << std::endl;
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
        std::cerr << "system error: sigaction failed with errno: " << errno <<
        std::endl;
        exit(1);
    }

    threads[0] = new Thread(nullptr, MAIN_THREAD);
    return 0;
}

int uthread_spawn(void (*f)(void)) {
    // verify the array of threads is not full, and find the minimal id to spawn
    for (unsigned int i = 0; i < MAX_THREAD_NUM; ++i) {
        if (threads[i] == nullptr) {
            threads[i] = new Thread(&f, i);
            readyThreads.insert(readyThreads.begin(), threads[i]);
            return i;
        }
        // no room for another thread.
        return -1;
    }
}
int uthread_block(int tid) {
    if (checkTidLegallity(tid)) {
        return -1;
    }

    if (threads[tid]->getStatus() == Blocked ||
        threads[tid]->getStatus() == Sleeping) {
        return 0;
    }

    threads[tid]->setStatus(Blocked);

    // the tid is the id of the running thread
    if (tid == uthread_get_tid()) {
        blockedThreads.insert(blockedThreads.begin(), runningThread);
        timer_handler(0); //finish the quanta

    } else { // the thread is in the ready vector
        for (int i = 0; i < readyThreads.size(); i++) {
            if (readyThreads[i]->getId() == tid) {
                readyThreads.erase(readyThreads.begin() + i);
                blockedThreads.insert(blockedThreads.begin(), threads[tid]);
                break;
            }
        }
    }
    return 0;
}

int uthread_resume(int tid) {
    if (threads[tid]->getStatus() != Blocked) {
        if (checkTidLegallity(tid)) {
            return -1;
        }

        threads[tid]->setStatus(Ready);

        for (int i = 0; i < blockedThreads.size(); i++) {
            if (blockedThreads[i]->getId() == tid) {
                blockedThreads.erase(blockedThreads.begin() + i);
                readyThreads.insert(blockedThreads.begin(), threads[tid]);
                break;
            }
        }
    }
    return 0;
}

int uthread_sleep(int num_quantums) {
    if (num_quantums <= 0) {
        std::cerr << "thread library error: parameter num_quantums must be a "
                "positive number" << std::endl;
        return -1;
    }
    if (runningThread->getId() == 0) {
        std::cerr << "thread library error: the main thread can't be put to "
                "sleep" << std::endl;
        return -1;
    }

    runningThread->setStatus(Sleeping);
    blockedThreads.insert(blockedThreads.begin(), runningThread);
    timer_handler(0); //finish the quanta
    return 0;
}

int checkTidExists(int tid) {
    if (threads[tid] == nullptr || tid > MAX_THREAD_NUM - 1) {
        std::cerr << "thread library error: parameter tid must be refer to an "
                "id of an existing thread" << std::endl;
        return -1;
    }
    return 0;
}

int uthread_get_time_until_wakeup(int tid) {
    // if the tid is not legal the return -1
    if (checkTidExists(tid)) {
        return -1;
    }
    if (threads[tid]->getStatus() == Sleeping) {
        if (tid == MAIN_THREAD) {
            return 0;
        }
        return threads[tid]->getSleepingCountdown();
    }
    //thread is not sleeping- return zero
    return 0;

}

int uthread_get_tid() {
    return runningThread->getId();
}

int uthread_get_total_quantums() {
    return generalQuantaCounter;
}

int uthread_get_quantums(int tid) {
    if (checkTidExists(tid)) {
        return -1;
    }

    // if the thread is the running thread then the current quantum should be
    // included.
    if (tid == uthread_get_tid()) {
        return (*threads[tid]).getQuantaCounter() + 1;
    }
    else {
        return (*threads[tid]).getQuantaCounter();
    }
}


// todo when does terminate return -1?
// todo any more allocated memory needs to be free?
int uthread_terminate(int tid) {
    // if id is of the running thread, change status to Terminate and
    if (tid == runningThread->getId()) {
        terminateThread = runningThread;
        timer_handler(SIGVTALRM);
    } else if (tid != 0) {
        // remove pointers to the thread object from blocked/sleeping/ready list
        switch (threads[tid]->getStatus()) {
            case Ready:
                for (int i = 0; i < readyThreads.size(); ++i) {
                    if (readyThreads[i]->getId() == tid) {
                        readyThreads.erase(readyThreads.begin() + i);
                    }
                }
                break;

            case Sleeping:
                for (int i = 0; i < sleepingThreads.size(); ++i) {
                    if (sleepingThreads[i]->getId() == tid) {
                        sleepingThreads.erase(sleepingThreads.begin() + i);
                    }
                }
                break;

            case Blocked:
                for (int i = 0; i < blockedThreads.size(); ++i) {
                    if (blockedThreads[i]->getId() == tid) {
                        blockedThreads.erase(blockedThreads.begin() + i);
                    }
                }
                break;

                // do nothing. just to make sure all cases are handled.
            case Running:
                break;
        }
        // delete the thread object after all of it's pointers were removed
        delete threads[tid];
        return 0;
    } else {
        for (int i = 1; i < MAX_THREAD_NUM; ++i) {
            if (threads[i] != nullptr) {
                delete threads[i];
            }
        }

        exit(0);
    }
    return 0;
}
