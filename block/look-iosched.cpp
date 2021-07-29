/*
 * elevator look
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>



struct look_data 
{
	enum seek_direction 
	{
		NONE,
		LEFT,
		RIGHT
	};

	struct list_head queue;


	seek_direction direction;
};

void sort_queue(struct request_queue *q)
{
	// Go to the beginnig of the queue.

}

static void look_merged_requests(struct request_queue* q, struct request* rq,
	struct request* next)
{
	list_del_init(&next->queuelist);
}

static int look_dispatch(struct request_queue* q, int force)
{
	struct look_data* nd = q->elevator->elevator_data;;

	if (nd->queue) 
	{
		if (nd->direction == LEFT) 
		{
			
		}
		if (nd->direction == RIGHT) 
		{
			list_del_init(nd->queue->queuelist);
			elv_dispatch_sort(q, nd->queue);
		}
		return 1;
	}
	return 0;
}

static void look_add_request(struct request_queue* q, struct request* rq)
{
	struct look_data* nd = q->elevator->elevator_data;

	// todo add in the right position.
}

static int look_init_queue(struct request_queue* q, struct elevator_type* e)
{
	struct look_data* nd;
	struct elevator_queue* eq;

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = nd;

	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);
	return 0;
}

static void look_exit_queue(struct elevator_queue* e)
{
	struct look_data* nd = e->elevator_data;

	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

static struct elevator_type elevator_look = {
	.ops.sq = {
		.elevator_merge_req_fn = look_merged_requests,
		.elevator_dispatch_fn = look_dispatch,
		.elevator_add_req_fn = look_add_request,
		.elevator_former_req_fn = look_former_request,
		.elevator_latter_req_fn = look_latter_request,
		.elevator_init_fn = look_init_queue,
		.elevator_exit_fn = look_exit_queue,
	},
	.elevator_name = "look",
	.elevator_owner = THIS_MODULE,
};

static int __init look_init(void)
{
	return elv_register(&elevator_look);
}

static void __exit look_exit(void)
{
	elv_unregister(&elevator_look);
}

module_init(look_init);
module_exit(look_exit);


MODULE_AUTHOR("João Mororo & Vitor Morais");
MODULE_LICENSE("MIT");
MODULE_DESCRIPTION("Look IO scheduler");