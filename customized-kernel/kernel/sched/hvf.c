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


long compute_init_sched_value(struct task_struct *p);
long reduce_sched_value(struct sched_hvf_entity *se);
void init_sched_hvf_entity(struct sched_hvf_entity *se);
bool hvf_rq_rbtree_insert(struct rb_root *root, struct sched_hvf_entity *se);
bool exceeded_time(struct task_struct *p);
void update_latest_se_hvf(struct sched_hvf_entity *se);
void update_used_se_hvf(struct sched_hvf_entity *se);
bool hvf_rq_empty(struct hvf_rq *hvf_rq);
long time_slice(long sched_value);



const struct sched_class hvf_sched_class;

static inline
struct sched_hvf_entity *pick_entity_hvf(struct hvf_rq *hvf_rq){
	return hvf_rq->max_value_entity;
}

static struct task_struct *pick_task_hvf(struct rq *rq){
	struct sched_hvf_entity *se_hvf;
	struct hvf_rq *hvf_rq;

	hvf_rq = &rq->hvf;
	if(hvf_rq_empty(hvf_rq))
		return NULL;
	se_hvf = pick_entity_hvf(hvf_rq);
	return task_hvf_of(se_hvf);
}


static struct task_struct *pick_next_task_hvf(struct rq *rq, struct task_struct *prev){
	if(prev->sched_class == &hvf_sched_class){
		struct sched_hvf_entity *prev_hvf = &prev->hvf;
		update_used_se_hvf(prev_hvf);
	}

	struct task_struct *next = pick_task_hvf(rq);
	if(!next)
		return NULL;
	return next;
}

static void enqueue_hvf_entity(struct hvf_rq *hvf_rq, struct sched_hvf_entity *se){
	hvf_rq_rbtree_insert(&hvf_rq->hvf_task_queue, se);
	se->on_rq = true;
	struct sched_hvf_entity *max_entity = hvf_rq->max_value_entity;

	if(!max_entity){
		hvf_rq->max_value_entity = rb_entry(rb_last(&hvf_rq->hvf_task_queue), struct sched_hvf_entity, run_node);
		return;
	}

	if(max_entity->curr_sched_value < se->curr_sched_value)
		hvf_rq->max_value_entity = rb_entry(rb_last(&hvf_rq->hvf_task_queue), struct sched_hvf_entity, run_node);
}

static void
enqueue_task_hvf(struct rq *rq, struct task_struct *p, int flags){
	struct hvf_rq *hvf_rq = &rq->hvf;
	struct sched_hvf_entity *se_hvf = &p->hvf;


	if(flags & (ENQUEUE_INITIAL | ENQUEUE_CHANGED)){
		init_sched_hvf_entity(se_hvf);
		compute_init_sched_value(p);
	}
	else{
		if(flags & (ENQUEUE_WAKEUP | ENQUEUE_MIGRATED | ENQUEUE_RESTORE))
			reduce_sched_value(se_hvf);
		if(exceeded_time(p) && (se_hvf != hvf_rq->curr) && pid_alive(p)){
			send_sig(SIGKILL, p, 0);
			return;
		}
	}

	if(se_hvf != hvf_rq->curr)
		enqueue_hvf_entity(hvf_rq, se_hvf);
}


static bool dequeue_hvf_entity(struct hvf_rq *hvf_rq, struct sched_hvf_entity *se){
	struct rb_node *max_node = &hvf_rq->max_value_entity->run_node;

	if(max_node == &se->run_node){
		struct rb_node *prev_node = rb_prev(&se->run_node);
		hvf_rq->max_value_entity = (prev_node!=NULL)? rb_entry(prev_node, struct sched_hvf_entity, run_node) : NULL;
	}

	rb_erase(&se->run_node, &hvf_rq->hvf_task_queue);
	se->on_rq=false;

	return true;
}


static bool
dequeue_task_hvf(struct rq *rq, struct task_struct *p, int flags){
	struct hvf_rq *hvf_rq = &rq->hvf;
	struct sched_hvf_entity *se_hvf = &p->hvf;

	if(se_hvf != hvf_rq->curr)
		return dequeue_hvf_entity(hvf_rq, se_hvf);
	return false;
}

static void
set_next_hvf_entity(struct hvf_rq *hvf_rq, struct sched_hvf_entity *se){
	if(se->on_rq)
		dequeue_hvf_entity(hvf_rq, se);

	update_latest_se_hvf(se);

	hvf_rq->curr = se;
}


static void set_next_task_hvf(struct rq *rq, struct task_struct *p, bool first){
	struct sched_hvf_entity *se_hvf = &p->hvf;
	struct hvf_rq *hvf_rq = &rq->hvf;

	set_next_hvf_entity(hvf_rq, se_hvf);
}


static void
switched_to_hvf(struct rq *rq, struct task_struct *p){
	// TODO
}

static void
switched_from_hvf(struct rq *rq, struct task_struct *p){
	// TODO
}

static void task_tick_hvf(struct rq *rq, struct task_struct *curr, int queued){
	struct sched_hvf_entity *curr_ent = &curr->hvf;
	long slice = time_slice(curr_ent->curr_sched_value);

	curr_ent->runtime += TICK_NSEC;

	if(curr_ent->runtime >= slice*K*K)
		resched_curr(rq);

}

static void task_dead_hvf(struct task_struct *p){
	// TODO
}

static void
put_prev_hvf_entity(struct hvf_rq *hvf_rq, struct sched_hvf_entity *se){
	if(!se->on_rq)
		enqueue_hvf_entity(hvf_rq, se);

	hvf_rq->curr = NULL;
}

static void put_prev_task_hvf(struct rq *rq, struct task_struct *prev, struct task_struct *next){
	struct sched_hvf_entity *se_hvf = &prev->hvf;
	struct hvf_rq *hvf_rq = &rq->hvf;

	put_prev_hvf_entity(hvf_rq, se_hvf);
}



DEFINE_SCHED_CLASS(hvf)={
	.enqueue_task = enqueue_task_hvf,
	.pick_task = pick_task_hvf,
	.dequeue_task = dequeue_task_hvf,
	.set_next_task = set_next_task_hvf,
	.pick_next_task = pick_next_task_hvf,
	.switched_to = switched_to_hvf,
	.switched_from = switched_from_hvf,
	.task_tick = task_tick_hvf,
	.task_dead = task_dead_hvf,
	.put_prev_task = put_prev_task_hvf
};



inline void init_hvf_rq(struct hvf_rq *hvf_rq){
	hvf_rq->hvf_task_queue = RB_ROOT;
	hvf_rq->max_value_entity = NULL;
}

inline long compute_init_sched_value(struct task_struct *p){
	const long first_time = p->hvf.first_time;
	const long D1 = p->deadline_1*K;
	const long D2 = p->deadline_2*K;
	const long X = first_time+p->computation_time;
	const long V = (X<D1)? H : (D2<X)? 0 : (D2-X)*H/(D2-D1);

	p->hvf.init_sched_value = V;
	p->hvf.curr_sched_value = V;

	return V;
}


inline long reduce_sched_value(struct sched_hvf_entity *se){
	long rate = se->init_sched_value;
	long old_value = se->curr_sched_value;
	long new_value = old_value - (rate*old_value/100);
	new_value = (new_value>0)? new_value : (old_value == 0)? 10: 0;

	se->curr_sched_value = new_value;

	return new_value;
}


bool hvf_rq_rbtree_insert(struct rb_root *root, struct sched_hvf_entity *se){
	struct rb_node **new_node = &(root->rb_node), *parent = NULL;

	while(*new_node!=NULL){
		struct sched_hvf_entity *this_entity = container_of(*new_node, struct sched_hvf_entity, run_node);
		int result = se->curr_sched_value - this_entity->curr_sched_value;

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
	se->runtime = 0;
}


inline bool hvf_rq_empty(struct hvf_rq *hvf_rq){
	struct rb_root *root = &hvf_rq->hvf_task_queue;
	return RB_EMPTY_ROOT(root);
}


/*this function shall be called in pick_next_task for the previous task*/

inline void update_used_se_hvf(struct sched_hvf_entity *se){
	struct timespec64 now;
	ktime_get_real_ts64(&now);

	long current_time = now.tv_sec*K + now.tv_nsec/(K*K);
	se->time_used += current_time - se->latest_time;
}

long time_slice(long sched_value){
	const int max_slice = K;
	const int min_slice = 10;

	if(sched_value < 1)
		sched_value = 1;
	if(sched_value > K)
		sched_value = K;
	long scaled_value = K-sched_value;
	long range = max_slice - min_slice;

	return (((scaled_value*range)/K)+min_slice);
}



