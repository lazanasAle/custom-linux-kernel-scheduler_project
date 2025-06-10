#include <linux/sched.h>
#include <linux/rbtree.h>
#include <linux/cpumask.h>
#include <asm-generic/errno.h>
#include "sched.h"


void init_hvf_rq(struct hvf_rq *hvf_rq){
	hvf_rq->hvf_task_queue = RB_ROOT;
	hvf_rq->max_value_entity = NULL;

}


const struct sched_class hvf_sched_class;





DEFINE_SCHED_CLASS(hvf)={

};
