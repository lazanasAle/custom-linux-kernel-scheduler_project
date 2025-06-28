#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>
#include <sys/syscall.h>

#define DELAY 750
#define K 1000

#define SCHED_HVF 8

#define __NR_SET_SCHED_PARS     467
#define __NR_GET_SCHED_PARS     468
#define __NR_GET_SCHED_SCORE    469

#define set_sched_params(x, y, z)   syscall(__NR_SET_SCHED_PARS, x, y, z)
#define get_sched_params(x)         syscall(__NR_GET_SCHED_PARS, x)


double a = 1.1;

void core_delay(){
    unsigned long j;

    for (j = 0; j < 100000; j++) {
        a += sqrt(1.1)*sqrt(1.2)*sqrt(1.3)*sqrt(1.4)*sqrt(1.5);
        a += sqrt(1.6)*sqrt(1.7)*sqrt(1.8)*sqrt(1.9)*sqrt(2.0);
        a += sqrt(1.1)*sqrt(1.2)*sqrt(1.3)*sqrt(1.4)*sqrt(1.5);
        a += sqrt(1.6)*sqrt(1.7)*sqrt(1.8)*sqrt(1.9);
    }
}

void delay(int workload){
    int i;
    int total_workload = workload*DELAY;

    for (i = 0; i < total_workload; i++)
        core_delay();
}

void do_work(int workload){
    int pid = getpid();

    printf("process %d begins\n", pid);
    delay(workload);
    printf("process %d ends\n", pid);
}

int main(){
    pid_t procs[20];
    long now = time(NULL);

    for (int j=0; j<20; ++j){
        procs[j] = fork();
        if (procs[j]<0){
            printf("error in fork\n");
            return 0;
        }
        else if (procs[j] == 0){
            struct sched_param param = {.sched_priority=0};
            if (!set_sched_params(now+j+1, now+j+4, (j+2)*K)){
                sched_setscheduler(0, SCHED_HVF, &param);
                do_work(j+1);
            }
            else {
                printf("invalid scheduling params\n");
            }
            exit(0);
        }
    }

    return 0;
}
