#include <linux/sched.h>
#include <linux/rbtree.h>
#include <linux/cpumask.h>
#include <linux/types.h>
#include <linux/timekeeping.h>
#include <asm-generic/errno.h>
#include "sched.h"

#define M 1000
#define H 100


long compute_sched_value(struct task_struct *p);
bool hvf_rq_rbtree_insert(struct rb_root *root, struct sched_hvf_entity *se);



const struct sched_class hvf_sched_class;





DEFINE_SCHED_CLASS(hvf)={

};



void init_hvf_rq(struct hvf_rq *hvf_rq){
	hvf_rq->hvf_task_queue = RB_ROOT;
	hvf_rq->max_value_entity = NULL;

}

long compute_sched_value(struct task_struct *p){
	struct timespec64 now;
	ktime_get_real_ts64(&now);
	const long D1 = p->deadline_1*M;
	const long D2 = p->deadline_2*M;
	const long X = now.tv_sec*M+p->computation_time;
	const long V = (X<D1)? H : (D2<X)? 0 : (D2-X)*H/(D2-D1);

	p->hvf.sched_value = V;

	return V;
}


bool hvf_rq_rbtree_insert(struct rb_root *root, struct sched_hvf_entity *se){
	struct rb_node **new_node = &(root->rb_node), *parent = NULL;

	while(*new_node!=NULL){
		struct sched_hvf_entity *this_entity = container_of(*new_node, struct sched_hvf_entity, run_node);
		int result = se->sched_value - this_entity->sched_value;

		parent = *new_node;
		if(result<=0)
			new_node = &((*new_node)->rb_left);
		else
			new_node = &((*new_node)->rb_right);
	}

	rb_link_node(&se->run_node, parent, new_node);
	rb_insert_color(&se->run_node, root);

	return true;
}
