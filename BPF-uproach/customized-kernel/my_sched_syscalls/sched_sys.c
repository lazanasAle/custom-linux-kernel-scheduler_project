#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/my_sched_sys.h>
#include <asm/current.h>
#include <asm-generic/errno.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/timekeeping.h>


#define M 1000
#define H 100

SYSCALL_DEFINE3(set_scheduling_params, long, deadline_1, long, deadline_2, long, computation_time){
	bool condition_1 = (deadline_1>0 && deadline_2>0) && deadline_1<deadline_2;
	struct timespec64 now;
	ktime_get_real_ts64(&now);

	bool condition_2 = (now.tv_sec*M+computation_time)<deadline_2*M;

	if(condition_1 && condition_2){
		current->deadline_1=deadline_1;
		current->deadline_2=deadline_2;
		current->computation_time=computation_time;

		return 0;
	}
	return -EINVAL;
}


SYSCALL_DEFINE1(get_scheduling_params, struct d_params *, params){
	bool condition = (params!=NULL) && access_ok(params, sizeof(struct d_params));
	if(condition){
		if(copy_to_user(&params->deadline_1, &current->deadline_1, sizeof(long))!=0){
			return -EFAULT;
		}
		if(copy_to_user(&params->deadline_2, &current->deadline_2, sizeof(long))!=0){
			return -EFAULT;
		}
		if(copy_to_user(&params->computation_time, &current->computation_time, sizeof(long))!=0){
			return -EFAULT;
		}
		return 0;
	}
	return -EINVAL;
}


SYSCALL_DEFINE0(get_scheduling_score){
	struct timespec64 now;
	ktime_get_real_ts64(&now);
	long D1 = current->deadline_1;
	long D2 = current->deadline_2;
	long D3 = now.tv_sec;

	if(D3<D1)
		return H;
	else if(D2<D3)
		return 0;
	else
		return ((D2-D3)*100)/(D2-D1);
}
