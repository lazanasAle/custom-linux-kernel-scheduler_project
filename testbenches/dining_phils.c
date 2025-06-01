#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sched.h>
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>

#define PHIL_COUNT 20

sem_t chopstick_sem[PHIL_COUNT];
pthread_mutex_t print_mtx;

void atomic_eat(size_t phil_id){
    pthread_mutex_lock(&print_mtx);
    printf("philosopher: %ld eating now\n", phil_id);
    sleep(3);
    printf("philosopher: %ld done eating now\n", phil_id);
    pthread_mutex_unlock(&print_mtx);
}


void* philosopher_thread(void* id_findexes){
    pid_t tid = syscall(SYS_gettid);
    struct sched_param param;

    size_t* ph_id = id_findexes;
    size_t philosopher_id=ph_id[0];
    size_t left_idx=ph_id[1];
    size_t right_idx=ph_id[2];
    size_t sched_pol=ph_id[3];
    if(sched_pol==SCHED_RR)
        param.sched_priority=philosopher_id+9;
    else
        param.sched_priority=0;

    if(sched_setscheduler(tid, sched_pol, &param)<0){
        perror("error setting scheduler\n");
        return nullptr;
    }


    size_t first_idx=(left_idx<right_idx)? left_idx : right_idx;
    size_t last_idx=(left_idx<right_idx)? right_idx : left_idx;

    sem_wait(&chopstick_sem[first_idx]);
    sem_wait(&chopstick_sem[last_idx]);
    atomic_eat(philosopher_id);
    sem_post(&chopstick_sem[first_idx]);
    sem_post(&chopstick_sem[last_idx]);
    free(id_findexes);
    return nullptr;
}

void proc_run(size_t sched_constant){
    pthread_t philosophers[PHIL_COUNT];

    for(size_t j=0; j<PHIL_COUNT-1; ++j){
        size_t* id_l_r = malloc(4*sizeof(size_t));
        id_l_r[0]=j;
        id_l_r[1]=j;
        id_l_r[2]=j+1;
        id_l_r[3]=sched_constant;
        pthread_create(&philosophers[j], nullptr, philosopher_thread, id_l_r);
    }
    size_t* id_l_r = malloc(4*sizeof(size_t));

    id_l_r[0]=PHIL_COUNT-1;
    id_l_r[1]=PHIL_COUNT-1;
    id_l_r[2]=0;
    id_l_r[3]=sched_constant;
    pthread_create(&philosophers[PHIL_COUNT-1], nullptr, philosopher_thread, id_l_r);

    for(size_t j=0; j<PHIL_COUNT; ++j){
        pthread_join(philosophers[j], nullptr);
    }
}

int main(){
    for(size_t j=0; j<PHIL_COUNT; ++j){
        sem_init(&chopstick_sem[j], 0, 1);
    }
    pthread_mutex_init(&print_mtx, nullptr);
    pid_t pid=fork();
    if(pid==0){
        struct timespec begin;
        clock_gettime(CLOCK_MONOTONIC, &begin);
        proc_run(SCHED_RR);
        struct timespec ending;
        clock_gettime(CLOCK_MONOTONIC, &ending);
        double passed = (ending.tv_sec-begin.tv_sec)+(ending.tv_nsec-begin.tv_nsec)/1e9;
        printf("time passed RR: %.3f\n", passed);
        exit(0);
    }
    else{
        wait(nullptr);
        struct timespec begin;
        clock_gettime(CLOCK_MONOTONIC, &begin);
        proc_run(SCHED_OTHER);
        struct timespec ending;
        clock_gettime(CLOCK_MONOTONIC, &ending);
        double passed = (ending.tv_sec-begin.tv_sec)+(ending.tv_nsec-begin.tv_nsec)/1e9;
        printf("time passed CFS: %.3f\n", passed);
        exit(0);
    }


}
