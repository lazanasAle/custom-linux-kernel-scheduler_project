#include <stdio.h>
#include <unistd.h>
#include<time.h>
#include <linux/my_sched_sys.h>
#include <sys/syscall.h>

#define __NR_SET_SCHED_PARS 467
#define __NR_GET_SCHED_PARS 468
#define __NR_GET_SCHED_SCORE 469


int main(){
    struct d_params pars;
    long ret=syscall(__NR_SET_SCHED_PARS, time(NULL)+3, time(NULL)+9, 400);
    if(ret){
        printf("invalid params \n");
        return -1;
    }
    long ret2=syscall(__NR_SET_SCHED_PARS, time(NULL)+11, time(NULL)+9, 400);    syscall(__NR_GET_SCHED_PARS, &pars);
    printf("d1: %ld, d2: %ld, d3: %ld\n", pars.deadline_1, pars.deadline_2, pars.computation_time);
    long cc=syscall(__NR_GET_SCHED_SCORE);
    printf("%ld\n", cc);
    sleep(9);
    long cc2=syscall(__NR_GET_SCHED_SCORE);
    printf("%ld\n", cc2);
    if(ret2){
        printf("invalid params \n");
        return -1;
    }


}
