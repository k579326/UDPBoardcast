
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "Gqueue.h"


struct node
{
    void* data;
	unsigned int datalen;
	
    struct node* prev;
    struct node* next;
};


struct easy_queue_t
{
    int size;
    struct node* queue_head;
    struct node* queue_tail;
	datafree 	 user_handler;
};




int easy_queue_init(struct easy_queue_t** queue, datafree free_cb)
{
    *queue = (easy_queue_t*)malloc(sizeof(easy_queue_t));
    (*queue)->size = 0;
	
    struct node* p = malloc(sizeof(struct node));
    p->data = NULL;
	p->datalen = 0;
    p->next = NULL;
    p->prev = NULL;
	
    (*queue)->queue_head = p;
    (*queue)->queue_tail = p;
	(*queue)->user_handler = free_cb;
	
    return 0;
}

int easy_queue_push(struct easy_queue_t* queue, const void* data, unsigned int datalen)
{
    struct node* p = malloc(sizeof(struct node));

    p->data = malloc(datalen);
	p->datalen = datalen;
	memcpy(p->data, data, datalen);
	
    p->next = NULL;
    queue->queue_tail->next = p;
    p->prev = queue->queue_tail;
    queue->queue_tail = p;
    queue->size++;

    return 0;
}

int easy_queue_pop(struct easy_queue_t* queue, void* buf, unsigned int* buflen)
{
	int ret = 0;
    struct node* p = queue->queue_head->next;

    if (p == NULL)
    {// queue is empyt
    	*buflen = 0;
        return -1;
    }

	if (!buf || *buflen < p->datalen)
    {
        *buflen = p->datalen;
		return -1;
    }

    queue->queue_head->next = p->next;
    if (p->next)
    {
        p->next->prev = queue->queue_head;
    }
    else
    {
        queue->queue_tail = queue->queue_head;
    }
	
	memcpy(buf, p->data, p->datalen);
	*buflen = p->datalen;
	
	free(p->data);
    free(p);

    queue->size--;
    return 0;
}

int easy_queue_clear(struct easy_queue_t* queue)
{
	struct node* cursor = NULL;
	
	// 队尾不可能为NULL
	while(queue->queue_tail != queue->queue_head)
	{
		cursor = queue->queue_tail;
		queue->queue_tail = queue->queue_tail->prev;
		queue->queue_tail->next = NULL;

		if (queue->user_handler)
		{
			queue->user_handler(cursor->data, cursor->datalen);
		}
		
		free(cursor->data);
		free(cursor);
		cursor = NULL;
		queue->size--;
	}

	assert(queue->size == 0);

	return 0;
}



int easy_queue_uninit(struct easy_queue_t* queue)
{
    free(queue->queue_head);
    free(queue);
    queue = NULL;

    return 0;
}

int easy_queue_size(struct easy_queue_t* queue)
{
    return queue->size;
}


