/*
 * elevator look
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

enum seek_direction { DIR_LEFT, DIR_RIGHT };

struct look_data 
{
	struct list_head queue;
	seek_direction direction;
};

static void look_merged_requests(struct request_queue* q, struct request* rq,
	struct request* next)
{
	list_del_init(&next->queuelist);
}

static int look_dispatch(struct request_queue* q, int force)
{
	struct look_data* nd = q->elevator->elevator_data;

	if (!list_empty(&nd->queue)) 
	{
		struct request *rq;
		if (nd->direction == DIR_LEFT) 
			rq = list_entry(nd->queue.prev, struct request, queuelist);
		else
			rq = list_entry(nd->queue.next, struct request, queuelist);

		list_del_init(&rq->queuelist);
		elv_dispatch_sort(q, rq);

		// Look ahead
		if (nd->direction == DIR_LEFT && nd->queue->next == NULL)
			nd->direction = DIR_RIGHT;
		if (nd->direction == DIR_RIGHT && nd->queue->prev == NULL)
			nd->direction = DIR_LEFT;

		return 1;
	}
	return 0;
}

static void look_add_request(struct request_queue* q, struct request* rq)
{
	struct look_data* nd = q->elevator->elevator_data;
	struct request *current_rq = list_entry(pos, struct request, queuelist);
	struct list_head *pos = NULL;

	seek_direction direction;
	struct sector_t disk_head = blk_rq_pos(current_rq) + blk_rq_sectors(current_rq);
	
	list_for_each (pos, &nd->queue)
	{
		current_rq = list_entry(pos, struct request, queuelist);
		if (blk_rq_pos(rq) < disk_head) 
			// Request is bigger than disk head
			if (blk_rq_pos(current_rq) < disk_head && blk_rq_pos(rq) < blk_rq_pos(current_rq)) 
			{
				direction = DIR_RIGHT;
				rq->direction = DIR_RIGHT;
				break;
			}
		else // Request is bigger than disk head
			// Find sport where current is smaller than the head or current smaller than request.
			if (blk_rq_pos(current_rq) < disk_head || blk_rq_pos(rq) < blk_rq_pos(current_rq)) 
			{
				direction = DIR_LEFT;
				rq->q->elevator->elevator_data->direction = DIR_LEFT;
				break;
			}
		if (!list_empty(&nd->queue)
			nd->direction = direction;
		list_add_tail(&rq->queuelist, pos);
	}
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
	INIT_LIST_HEAD(&nd->queue);

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