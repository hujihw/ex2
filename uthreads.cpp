#include <sys/time.h>
#include <csetjmp>
#include <cstdio>
#include <signal.h>
#include <vector>
#include <errno.h>
#include <iostream>
#include <list>
#include "uthreads.h"
#include <unistd.h>

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

enum status_t {Ready, Blocked, Sleeping, Running}; // todo move other declerations here?

////////////////////
/// class Thread ///
////////////////////


class Thread {
//    using namespace uthreads_utils;

public:
    Thread(void (*entry_point)(), const unsigned int id);
    ~Thread();
    sigjmp_buf env;
    void quantaCounterUp();
    void setStatus(status_t new_status);
    const status_t getStatus() const;
    unsigned int getId() const;
    unsigned int getQuantaCounter() const;
    int getSleepingCountdown() const;
    void setSleepingCountdown(int sleepingCountdown); // todo remove
    void decreaseSleepingCountdown();

private:
    int sleepingCountdown;
    unsigned int id;
    status_t status;
    unsigned int quantaCounter = 0;
    char *stack;
    void (*entry_point); // todo remove
};

//////////////////////
/// Uthreads Utils ///
//////////////////////

itimerval timer;
struct sigaction sig_handler;
unsigned int generalQuantaCounter = 1;
Thread *threads[MAX_THREAD_NUM];
std::vector<Thread*> readyThreads; // todo check better option than vector
std::vector<Thread*> blockedThreads;
std::vector<Thread*> sleepingThreads; // todo remove if not needed
Thread* runningThread;
Thread* terminateThread = nullptr;
int quantumLength;
sigset_t set; //the signal set for sigporcmask

void timer_handler(int sig);

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

//block the SIGVTALRM
void blockSigvtalrm(){
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    if (sigprocmask(SIG_SETMASK, &set, NULL)){
        std::cerr << "system error: sigprocmask failed with errno: " <<
        errno << std::endl;
        exit(1);
    }
}

//unblock the SIGVTALRM
void unBlockSigvtalrm(){
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    if (sigprocmask(SIG_UNBLOCK, &set, NULL)){
        std::cerr << "system error: sigprocmask failed with errno: " <<
        errno << std::endl;
        exit(1);
    }
}

// ignore SIGVTALRM
void ignoreSigvtalrm() {
    sig_handler.sa_handler = SIG_IGN;
    sig_handler.sa_flags = 0;
    if (sigaction(SIGVTALRM, &sig_handler, nullptr)){
        std::cerr << "system error: sigaction failed with errno: " <<
        errno << std::endl;
        exit(1);
    }
}

// un ignore SIGVTALRM
void unIgnoreSigvtalrm() {
    sig_handler.sa_handler = timer_handler;
    sig_handler.sa_flags = 0;
    if (sigaction(SIGVTALRM, &sig_handler, nullptr)){
        std::cerr << "system error: sigaction failed with errno: " <<
        errno << std::endl;
        exit(1);
    }
}

void timer_handler(int sig) {
    ignoreSigvtalrm();
    int previousRunningThread = runningThread->getId();

    generalQuantaCounter += 1;

    ////////////////////////
    /// Round Robin alg. ///
    ////////////////////////

    // add the threads that finished sleeping to ready vector
    wakeupSleepingThreads();

    if (runningThread->getStatus() == Running){
        runningThread->setStatus(Ready);
        readyThreads.insert(readyThreads.begin(), runningThread);
    }
//    int ret_val = sigsetjmp(runningThread->env, 1);

    if (readyThreads.size() >= 1)
    {
        // first thread in ready vector become running
        runningThread = readyThreads.back();
        runningThread->setStatus(Running);
        readyThreads.pop_back();

        //increment the quanta counter of the running thread
        runningThread->quantaCounterUp();

        // terminate the thread that needs termination
        if (terminateThread != nullptr){
            int terminate = terminateThread->getId();
            terminateThread = nullptr;
            uthread_terminate(terminate);
        }

        unIgnoreSigvtalrm();

        if (threads[previousRunningThread] != nullptr){

            // instead of using ret_val
            if (sigsetjmp(threads[previousRunningThread]->env, 1)){
                setitimer(ITIMER_VIRTUAL, &timer, nullptr);
                return;
            }
        }


//        int ret_val = sigsetjmp(runningThread->env, 1); // todo remove

        // reset the timer
        setitimer(ITIMER_VIRTUAL, &timer, nullptr);
//        if (ret_val == 1) { // todo remove
//            return;
//        }
        siglongjmp(runningThread->env, 1);
    } else {

        //increment the quanta counter of the running thread
        runningThread->quantaCounterUp();

        unIgnoreSigvtalrm();

        // reset timer
        setitimer(ITIMER_VIRTUAL, &timer, nullptr);
    }
}


// if the threads blocks itself then ignore SIGVTALRM, else block SIGVTALRM
void blockOrIgnoreSigvtalrm(int tid){
    if (tid == uthread_get_tid()){
        ignoreSigvtalrm();
    } else {
        blockSigvtalrm();
    }
}

// if the threads blocks itself then unignore SIGVTALRM, else unblock SIGVTALRM
void unBlockOrUnIgnoreSigvtalrm(int tid){

    if (tid == uthread_get_tid()){
        unIgnoreSigvtalrm();
    } else {
        unBlockSigvtalrm();
    }
}



namespace uthreads_utils { // todo remove

}


//////////////////////////////
/// Thread Implementations ///
//////////////////////////////

Thread::Thread(void (*entry_point)(), const unsigned int id) {
    Thread::id = id;
    Thread::status = Ready;
    Thread::stack = new char[STACK_SIZE];

    address_t sp, pc;
    sp = (address_t)stack + STACK_SIZE - sizeof(address_t);
    pc = (address_t)entry_point;
    sigsetjmp(this->env, 1);
    (this->env->__jmpbuf)[JB_SP] = translate_address(sp);
    (this->env->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&(this->env->__saved_mask));
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

using namespace uthreads_utils; //todo remove if needed

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

int checkTidExists(int tid) {
    if (threads[tid] == nullptr || tid > MAX_THREAD_NUM - 1) {
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
    runningThread = threads[0];
    runningThread->setStatus(Running);

    //increment the quanta counter of the running thread
    runningThread->quantaCounterUp();

    return 0;
}

int uthread_spawn(void (*f)(void)) {

    // block SIGVTALRM
    blockSigvtalrm();

    // verify the array of threads is not full, and find the minimal id to spawn
    for (unsigned int i = 0; i < MAX_THREAD_NUM; ++i) {
        if (threads[i] == nullptr) {
            threads[i] = new Thread(f, i);
            readyThreads.insert(readyThreads.begin(), threads[i]);
            unBlockSigvtalrm();
            return i;
        }
    }
    unBlockSigvtalrm();
    // no room for another thread.
    return -1;
}

int uthread_block(int tid) {

    blockOrIgnoreSigvtalrm(tid);

    if (checkTidLegallity(tid)) {

        unBlockOrUnIgnoreSigvtalrm(tid);

        return -1;
    }

    if (threads[tid]->getStatus() == Blocked ||
        threads[tid]->getStatus() == Sleeping) {
        unBlockOrUnIgnoreSigvtalrm; //todo debug
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
        unBlockSigvtalrm();
    }
    return 0;
}

int uthread_resume(int tid) {
    blockSigvtalrm();

    if (threads[tid]->getStatus() != Blocked) {
        if (checkTidExists(tid)) {
            unBlockSigvtalrm();
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
    unBlockSigvtalrm();
    return 0;
}

int uthread_sleep(int num_quantums) {
    ignoreSigvtalrm();

    if (num_quantums < 0) {
        std::cerr << "thread library error: parameter num_quantums must be a "
                "positive number" << std::endl;
        unIgnoreSigvtalrm();
        return -1;
    }
    if (runningThread->getId() == 0) {
        std::cerr << "thread library error: the main thread can't be put to "
                "sleep" << std::endl;
        unIgnoreSigvtalrm();
        return -1;
    }

    runningThread->setStatus(Sleeping);
    blockedThreads.insert(blockedThreads.begin(), runningThread);
    timer_handler(0); //finish the quanta
    unIgnoreSigvtalrm();
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
//    if (tid == uthread_get_tid()) {
//        return threads[tid]->getQuantaCounter() + 1; // todo remove
//        return threads[tid]->getQuantaCounter();
//    }
//    else {
    return threads[tid]->getQuantaCounter();
//    }
}

int uthread_terminate(int tid) {
    if (checkTidExists(tid)){
        return -1;
    }

    // if id is of the running thread, change status to Terminate and
    if (tid == MAIN_THREAD) {
        ignoreSigvtalrm();

        for (int i = 1; i < MAX_THREAD_NUM; ++i) {
            if (threads[i] != nullptr) {
                delete threads[i];
            }
        }
        exit(0);

    } else if (tid == runningThread->getId()) {
        ignoreSigvtalrm();

        terminateThread = runningThread;
        timer_handler(SIGVTALRM);
    } else {
        blockSigvtalrm();

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
        threads[tid] = nullptr;

        unBlockSigvtalrm();
        return 0;
    }
    return 0;
}
