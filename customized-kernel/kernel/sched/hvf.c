#include <linux/sched.h>
#include <linux/rbtree.h>
#include <linux/cpumask.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/sched/signal.h>
#include <linux/timekeeping.h>
#include <asm-generic/errno.h>
#include "sched.h"

#define K 1000
#define H 100


long compute_sched_value(struct task_struct *p);
void init_sched_hvf_entity(struct sched_hvf_entity *se);
bool hvf_rq_rbtree_insert(struct rb_root *root, struct sched_hvf_entity *se);
bool exceeded_time(struct task_struct *p);
void update_latest_se_hvf(struct sched_hvf_entity *se);
void update_used_se_hvf(struct sched_hvf_entity *se);
bool hvf_rq_empty(struct hvf_rq *hvf_rq);



const struct sched_class hvf_sched_class;

static inline
struct sched_hvf_entity *pick_next_entity_hvf(struct hvf_rq *hvf_rq){
	return hvf_rq->max_value_entity;
}

static struct task_struct *pick_task_hvf(struct rq *rq){
	struct sched_hvf_entity *se_hvf;
	struct hvf_rq *hvf_rq;

	hvf_rq = &rq->hvf;
	if(hvf_rq_empty(hvf_rq))
		return NULL;
	se_hvf = pick_next_entity_hvf(hvf_rq);
	return task_hvf_of(se_hvf);
}

static void enqueue_entity(struct hvf_rq *hvf_rq, struct sched_hvf_entity *se){
	hvf_rq_rbtree_insert(&hvf_rq->hvf_task_queue, se);

	struct sched_hvf_entity *max_entity = hvf_rq->max_value_entity;

	if(!max_entity){
		hvf_rq->max_value_entity = rb_entry(rb_last(&hvf_rq->hvf_task_queue), struct sched_hvf_entity, run_node);
		return;
	}

	if(max_entity->sched_value < se->sched_value)
		hvf_rq->max_value_entity = rb_entry(rb_last(&hvf_rq->hvf_task_queue), struct sched_hvf_entity, run_node);
}

static void
enqueue_task_hvf(struct rq *rq, struct task_struct *p, int flags){
	struct hvf_rq *hvf_rq = &rq->hvf;
	struct sched_hvf_entity *se_hvf = &p->hvf;
	if(flags & (ENQUEUE_WAKEUP | ENQUEUE_INITIAL | ENQUEUE_MIGRATED | ENQUEUE_RESTORE))
		compute_sched_value(p);

	if(flags & (ENQUEUE_INITIAL | ENQUEUE_CHANGED))
		init_sched_hvf_entity(se_hvf);
	else{
		update_latest_se_hvf(se_hvf);

		if(exceeded_time(p) && (se_hvf != hvf_rq->curr) && pid_alive(p)){
			send_sig(SIGKILL, p, 0);
			return;
		}
	}

	if(se_hvf != hvf_rq->curr)
		enqueue_entity(hvf_rq, se_hvf);
}


static bool dequeue_entity(struct hvf_rq *hvf_rq, struct sched_hvf_entity *se){
	update_used_se_hvf(se);

	struct rb_node *max_node = &hvf_rq->max_value_entity->run_node;

	if(max_node == &se->run_node){
		struct rb_node *next_node = rb_next(&se->run_node);
		hvf_rq->max_value_entity = rb_entry(next_node, struct sched_hvf_entity, run_node);
	}

	rb_erase(&se->run_node, &hvf_rq->hvf_task_queue);

	return true;
}


static bool
dequeue_task_hvf(struct rq *rq, struct task_struct *p, int flags){
	struct hvf_rq *hvf_rq = &rq->hvf;
	struct sched_hvf_entity *se_hvf = &p->hvf;

	if(se_hvf != hvf_rq->curr)
		return dequeue_entity(hvf_rq, se_hvf);
	return false;
}



DEFINE_SCHED_CLASS(hvf)={
	.enqueue_task = enqueue_task_hvf,
	.pick_task = pick_task_hvf,
	.dequeue_task = dequeue_task_hvf
};



inline void init_hvf_rq(struct hvf_rq *hvf_rq){
	hvf_rq->hvf_task_queue = RB_ROOT;
	hvf_rq->max_value_entity = NULL;
}

inline long compute_sched_value(struct task_struct *p){
	struct timespec64 now;
	ktime_get_real_ts64(&now);
	const long D1 = p->deadline_1*K;
	const long D2 = p->deadline_2*K;
	const long X = now.tv_sec*K+now.tv_nsec/(K*K)+p->computation_time;
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

inline void init_sched_hvf_entity(struct sched_hvf_entity *se){
	struct timespec64 now;
	ktime_get_real_ts64(&now);
	se->first_time = now.tv_sec*K + now.tv_nsec/(K*K);
	se->latest_time = se->first_time;
	se->time_used=0;
}

inline bool exceeded_time(struct task_struct *p){
	struct sched_hvf_entity *se_hvf = &p->hvf;
	long used = se_hvf->time_used;
	long ctime = p->computation_time;

	return(used>ctime);
}


inline void update_latest_se_hvf(struct sched_hvf_entity *se){
	struct timespec64 now;
	ktime_get_real_ts64(&now);
	se->latest_time = now.tv_sec*K + now.tv_nsec/(K*K);
}


inline bool hvf_rq_empty(struct hvf_rq *hvf_rq){
	struct rb_root *root = &hvf_rq->hvf_task_queue;
	return RB_EMPTY_ROOT(root);
}


/*this function shall be called in dequeue_task_hvf right before dequeuing the entity*/

inline void update_used_se_hvf(struct sched_hvf_entity *se){
	struct timespec64 now;
	ktime_get_real_ts64(&now);

	long current_time = now.tv_sec*K + now.tv_nsec/(K*K);
	se->time_used += current_time - se->latest_time;
}



