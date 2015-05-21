#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "common.h"

#define IS_QUEUE_EMPTY(Q) ((Q).count == 0)

#define CREATE_QUEUE_TYPE(NAME, TYPE)           \
    typedef struct {                            \
        TYPE* head;                             \
        TYPE* tail;                             \
        uint32_t count;                         \
    } NAME;
#define QUEUE_INIT(Q){                          \
    Q.head = NULL;                              \
    Q.tail = NULL;                              \
    Q.count= 0;                                 \
}while(0)
#define QUEUE_POP_FRONT(Q, VALUE) {             \
        if(Q.count == 0) VALUE = NULL;          \
        else{                                   \
            VALUE = Q.head;                     \
            Q.head = Q.head->next;              \
            Q.count--;                          \
            if(Q.head==NULL){                   \
                Q.tail == NULL;                 \
            }                                   \
        }                                       \
    } while(0)

#define QUEUE_PUSH_BACK(Q, INPUT) {             \
            if(INPUT!=NULL){                    \
                (INPUT)->next= NULL;            \
                if(Q.count == 0){               \
                    Q.head = INPUT;             \
                    Q.tail = NULL;              \
                }else if(Q.count == 1){         \
                     Q.head->next = INPUT;      \
                    Q.tail = INPUT;             \
                }else{                          \
                    Q.tail->next = INPUT;       \
                    Q.tail = INPUT;             \
                }                               \
                Q.count++;                      \
            }                                   \
    } while(0)

#endif//__QUEUE_H__
