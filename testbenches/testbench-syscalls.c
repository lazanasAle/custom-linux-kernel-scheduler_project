#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <linux/my_sched_sys.h>

#define __NR_SET_SCHED_PARS 467
#define __NR_GET_SCHED_PARS 468
#define __NR_GET_SCHED_SCORE 469

#define GOOD_TIME 1200
#define BAD_TIME 120000

#define set_sched_params(x, y, z) syscall(__NR_SET_SCHED_PARS, x, y, z)
#define get_sched_params(x)       syscall(__NR_GET_SCHED_PARS, x)


void print_params(struct d_params *params);
void make_first_right();
void make_first_wrong();
void make_second_right();
void make_second_wrong();

int main(){
    make_first_right();

    long score_before = syscall(__NR_GET_SCHED_SCORE);
    printf("score before sleeping = %ld\n", score_before);
    sleep(4);
    long score_after4 = syscall(__NR_GET_SCHED_SCORE);
    printf("score after sleeping 4 = %ld\n", score_after4);

    sleep(3);
    long score_after7 = syscall(__NR_GET_SCHED_SCORE);
    printf("score after sleeping 7 = %ld\n", score_after7);

    sleep(2);
    long score_after9 = syscall(__NR_GET_SCHED_SCORE);
    printf("score after sleeping 9 = %ld\n", score_after9);

    sleep(4);
    long score_after13 = syscall(__NR_GET_SCHED_SCORE);
    printf("score after sleeping 13 = %ld\n", score_after13);

    make_first_wrong();

    make_second_right();

    make_second_wrong();
}


void print_params(struct d_params *params){
    printf("Deadline 1: %ld\n", params->deadline_1);
    printf("Deadline 2: %ld\n", params->deadline_2);
    printf("Computation Time: %ld\n", params->computation_time);
}


void make_first_right(){
    time_t now = time(NULL);
    long ret1=set_sched_params(now+7, now+12, GOOD_TIME);
    if(ret1!=0){
        printf("[-] incorrect rejection of parameters with values: D1: %ld D2: %ld CT: %ld\n", now+7, now+10, GOOD_TIME);
    }
    else{
        printf("[+] correctly accepted right parameters\n");
        struct d_params pars;
        get_sched_params(&pars);
        print_params(&pars);
    }
}

void make_first_wrong(){
    time_t now = time(NULL);
    long ret1=set_sched_params(now+11, now+8, GOOD_TIME);
    if(ret1!=0){
        printf("[+] correctly rejecting d2<d1 parameters\n");
    }
    else{
        printf("[-] the parameters were accepted incorrectly\n");
        struct d_params pars;
        get_sched_params(&pars);
        print_params(&pars);
    }
    long ret2=set_sched_params(now+7, now+13, BAD_TIME);
    if(ret1!=0){
        printf("[+] correctly rejecting parameters with computation_time more than given in deadlines\n");
    }
    else{
        printf("[-] the parameters were accepted incorrectly\n");
        struct d_params pars;
        get_sched_params(&pars);
        print_params(&pars);
    }
}

void make_second_right(){
    struct d_params *pars=malloc(sizeof(struct d_params));
    long ret_right1 = get_sched_params(pars);
    if(ret_right1!=0){
        printf("[-] incorrectly rejected a valid pointer\n");
    }
    else{
        printf("[+] the valid pointer has the values\n");
        print_params(pars);
    }
    free(pars);
}


void make_second_wrong(){
    struct d_params *pars;
    long ret_wrong1 = get_sched_params(pars);
    if(ret_wrong1!=0){
        printf("[+] correctly rejected an invalid pointer\n");
    }
    else{
        printf("[-] the invalid pointer surprisingly has the values\n");
        print_params(pars);
    }
    pars=NULL;
    ret_wrong1 = get_sched_params(pars);
    if(ret_wrong1!=0){
        printf("[+] correctly rejected a NULL pointer\n");
    }
    else{
        printf("[-] the NULL pointer surprisingly has the values\n");
    }
}
