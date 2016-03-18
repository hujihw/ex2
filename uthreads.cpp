#include <sys/time.h>
#include <csetjmp>
#include <cstdio>
#include <signal.h>
#include <vector>
#include "uthreads.h"

namespace uthreads_utils {
    enum status_t {Ready, Blocked, Sleeping, Running};
    Thread threads[MAX_THREAD_NUM];
    std::vector<Thread> readyThreads; // todo check better option than vector
    std::vector<Thread> blockedThreads;
    std::vector<Thread> sleepingThreads; // todo remove if not needed
}

int uthread_init(int quantum_usecs) {
    itimerval timer;
    sigaction sig_handler;

    // the scheduler mechanics
    void timer_handler(int sig)
    {
        // Block SIGVTALRM
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGVTALRM);
        sigprocmask(SIG_SETMASK, &set, NULL);

        //todo handle signal


        // unblock SIGVTALRM
        sigprocmask(SIG_BLOCK, &set, NULL);
    }

    // Install timer_handler as the signal handler for SIGVTALRM.
    sig_handler.sa_handler = &timer_handler;
    if (sigaction(SIGVTALRM, &sig_handler, NULL) < 0) {
        return -1;
    }

    // Configure the timer to expire after "quantum_usecs".
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = quantum_usecs;

    // configure the timer to not execute an interval.
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    // Start a virtual timer. It counts down whenever this process is executing.
    if (quantum_usecs <= 0 || setitimer(ITIMER_VIRTUAL, &timer, nullptr)) {
        return -1;
    }

    Thread main_thread;
    return 0;
}

/// class Thread
class Thread {
    using namespace uthreads_utils;

    // Attributes
private:
    status_t status;
    void *entry_point;

public:
    jmp_buf env;

    void setStatus (status_t new_status){
        Thread::status = new_status;
    }

    const status_t getStatus() const {
        return status;
    }
};
