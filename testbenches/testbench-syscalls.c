#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>
#include <linux/my_sched_sys.h>

#define __NR_SET_SCHED_PARS 467
#define __NR_GET_SCHED_PARS 468
#define __NR_GET_SCHED_SCORE 469

void print_sched_params(struct d_params *pars) {
    printf("Current Scheduling Parameters:\n");
    printf("  Deadline 1       : %ld\n", pars->deadline_1);
    printf("  Deadline 2       : %ld\n", pars->deadline_2);
    printf("  Computation Time : %ld\n", pars->computation_time);
}

int main() {
    struct d_params pars;
    time_t now = time(NULL);

    // Valid case
    long ret1 = syscall(__NR_SET_SCHED_PARS, now + 5, now + 15, 300);
    if (ret1 != 0) {
        perror("SET_SCHED_PARS failed (valid case)");
        return -1;
    }
    printf("[+] Set valid scheduling parameters.\n");

    // Fetch current scheduling parameters
    if (syscall(__NR_GET_SCHED_PARS, &pars) == 0) {
        print_sched_params(&pars);
    } else {
        perror("GET_SCHED_PARS failed");
        return -1;
    }

    // Get initial score
    long score1 = syscall(__NR_GET_SCHED_SCORE);
    if (score1 < 0) {
        perror("GET_SCHED_SCORE failed");
        return -1;
    }
    printf("Initial scheduling score: %ld\n", score1);

    // Wait a few seconds to simulate passage of time
    sleep(7);

    // Get score after wait
    long score2 = syscall(__NR_GET_SCHED_SCORE);
    printf("Scheduling score after 7s: %ld\n", score2);

    // Invalid case: deadline_1 > deadline_2
    long ret2 = syscall(__NR_SET_SCHED_PARS, now + 20, now + 10, 200);
    if (ret2 != 0) {
        printf("[+] Correctly rejected invalid parameters (d1 > d2).\n");
    } else {
        printf("[-] ERROR: Accepted invalid parameters (d1 > d2).\n");
        return -1;
    }

    // Invalid case: computation time exceeds deadline range
    long ret3 = syscall(__NR_SET_SCHED_PARS, now + 30, now + 40, 100000);
    if (ret3 != 0) {
        printf("[+] Correctly rejected invalid parameters (computation too long).\n");
    } else {
        printf("[-] ERROR: Accepted invalid computation time.\n");
        return -1;
    }

    return 0;
}
