#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "common.h"
/**
 * @brief Checks if a queue is empty
 * 
 * @param Q This should be an object of type NAME (this type should be created by CREATE_QUEUE_TYPE)
 * @return 1 if queue is empty, else 0
 */
#define IS_QUEUE_EMPTY(Q) ((Q).count == 0)

/**
 * @brief Creates a structure named NAME that can be used as a queue of pointers to TYPE objects
 * @details this macro uses the NAME passed as the assigned name for the struct that will
 * be created.
 * The struct type that is created can be used as a queue and QUEUE_PUSH_BACK and QUEUE_POP_BACK
 * can be used to add and remove elements from this queue
 * 
 * @param NAME The name to assign to the struct for this type
 * @param TYPE The type of structure that this queue will hold pointers for. 
 *      IMPORTANT: type is assumed to be a struct that has the following member:
 *          struct TYPE* next;
 * 
 */
#define CREATE_QUEUE_TYPE(NAME, TYPE)           \
    typedef struct {                            \
        TYPE* head;                             \
        TYPE* tail;                             \
        uint32_t count;                         \
    } NAME;
/**
 * @brief Initializes an instance of a queue
 * 
 * @param Q  An instance of a queue type (defined by CREATE_QUEUE_TYPE)
 */
#define QUEUE_INIT(Q){                          \
    Q.head = NULL;                              \
    Q.tail = NULL;                              \
    Q.count= 0;                                 \
} while(0)

/**
 * @brief removes the head of the queue and stores it's pointer in VALUE
 * 
 * @param Q     An instance of a queue type (defined by CREATE_QUEUE_TYPE)
 * @param VALUE a TYPE pointer to store the resulting address that is popped from the queue
 */
#define QUEUE_POP_FRONT_GENERIC(Q, VALUE, NEXT) {       \
        if(Q.count == 0) VALUE = NULL;                  \
        else{                                           \
            VALUE = Q.head;                             \
            Q.head = Q.head->NEXT;                      \
            Q.count--;                                  \
            if(Q.head==NULL){                           \
                Q.tail = NULL;                          \
            }                                           \
        }                                               \
    } while(0)

/**
 * @brief pushes a TYPE pointer onto the back of a queue object
 * 
 * @param Q     An instance of a queue type (defined by CREATE_QUEUE_TYPE)
 * @param INPUT the TYPE pointer to push onto the end of Q
 */
#define QUEUE_PUSH_BACK_GENERIC(Q, INPUT, NEXT) {       \
            if(INPUT!=NULL){                            \
                (INPUT)->NEXT= NULL;                    \
                if(Q.count == 0){                       \
                    Q.head = INPUT;                     \
                    Q.tail = INPUT;                     \
                }else{                                  \
                    Q.tail->NEXT = INPUT;               \
                    Q.tail = INPUT;                     \
                }                                       \
                Q.count++;                              \
            }                                           \
    } while(0)



#define QUEUE_POP_FRONT(Q, VALUE) {             \
    QUEUE_POP_FRONT_GENERIC(Q, VALUE, next);    \
} while(0)

#define QUEUE_PUSH_BACK(Q, VALUE) {             \
    QUEUE_PUSH_BACK_GENERIC(Q, VALUE, next);    \
} while(0)

#endif//__QUEUE_H__
