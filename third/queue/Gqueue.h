

#pragma once


/*!
*   Ë«Ïò¶ÓÁÐ
*
*   @data 2018-04-12
*   @author guok
*/
#ifdef __cplusplus
extern "C" {
#endif

typedef void (*datafree)(void* data, unsigned int len);
typedef struct easy_queue_t easy_queue_t;

int easy_queue_init(struct easy_queue_t** queue, datafree free_cb);

int easy_queue_push(struct easy_queue_t* queue, const void* data, unsigned int datalen);

int easy_queue_pop(struct easy_queue_t* queue, void* buf, unsigned int* buflen);

int easy_queue_clear(struct easy_queue_t* queue);

int easy_queue_uninit(struct easy_queue_t* queue);

int easy_queue_size(struct easy_queue_t* queue);



#ifdef __cplusplus
}
#endif


