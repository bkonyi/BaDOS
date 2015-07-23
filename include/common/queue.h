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
        TYPE* iterator_previous;                \
        TYPE* iterator;                         \
        uint32_t count;                         \
    } NAME;
/**
 * @brief Initializes an instance of a queue
 * 
 * @param Q  An instance of a queue type (defined by CREATE_QUEUE_TYPE)
 */
#define QUEUE_INIT(Q) {                           \
    (Q).head = NULL;                              \
    (Q).tail = NULL;                              \
    (Q).iterator = NULL;                          \
    (Q).iterator_previous = NULL;                 \
    (Q).count= 0;                                 \
} while(0)


#define QUEUE_VALUE_EXISTS_IN(Q,VALUE,RETVAL,NEXT_MEMBER) {\
    RETVAL = false ;\
            if((VALUE)!=NULL) {                                                  \
                (Q).iterator = (Q).head;                                         \
                (Q).iterator_previous = NULL;                                    \
                while((Q).iterator != NULL) {                                    \
                    if(VALUE == (Q).iterator) {                                  \
                        RETVAL = true;                                           \
                        break;                                                   \
                    }                                                            \
                    (Q).iterator_previous = (Q).iterator;                        \
                    (Q).iterator = (Q).iterator->NEXT_MEMBER;                    \
                }                                                                \
            }\
} while(0)

#define QUEUE_REMOVE_VALUE(Q, INPUT, NEXT_MEMBER,COMPARE_FUNCTION) {                              \
            if((INPUT)!=NULL) {                                                  \
                if(COMPARE_FUNCTION((Q).head ,INPUT)) {                                          \
                    (Q).iterator = (Q).head;                                     \
                    (Q).head = (Q).head->NEXT_MEMBER;                            \
                    (Q).iterator->NEXT_MEMBER = NULL;                            \
                }else{                                                           \
                    (Q).iterator = (Q).head;                                     \
                    (Q).iterator_previous = NULL;                                \
                    while((Q).iterator != NULL) {                                \
                        if(COMPARE_FUNCTION((Q).iterator, INPUT)) {               \
                            (Q).iterator_previous->NEXT_MEMBER = (Q).iterator->NEXT_MEMBER;\
                            (Q).iterator->NEXT_MEMBER = NULL;                    \
                            break;                                               \
                        }                                                        \
                        (Q).iterator_previous = (Q).iterator;                    \
                        (Q).iterator = (Q).iterator->NEXT_MEMBER;                \
                    } \
                }\
            }\
        }while(0)\


/**
 * @brief removes the head of the queue and stores it's pointer in VALUE
 * 
 * @param Q     An instance of a queue type (defined by CREATE_QUEUE_TYPE)
 * @param VALUE a TYPE pointer to store the resulting address that is popped from the queue
 */
#define QUEUE_POP_FRONT_GENERIC(Q, VALUE, NEXT) {       \
        if((Q).count == 0) {                            \
            VALUE = NULL;                               \
        } else {                                        \
            VALUE = (Q).head;                           \
            (Q).head = (Q).head->NEXT;                  \
            (Q).count--;                                \
            if((Q).head==NULL) {                         \
                (Q).tail = NULL;                        \
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
            if(INPUT!=NULL) {                           \
                (INPUT)->NEXT= NULL;                    \
                if((Q).count == 0) {                    \
                    (Q).head = INPUT;                   \
                    (Q).tail = INPUT;                   \
                } else {                                \
                    (Q).tail->NEXT = INPUT;             \
                    (Q).tail = INPUT;                   \
                }                                       \
                (Q).count++;                            \
            }                                           \
    } while(0)

/**
 * @brief Treats the queue as a sorted queue. This , of course, assumes that the queue
 * is already sorted. To maintain this, ONLY USER QUEUE_SORTED_INSERT on a queue that you
 * inted to be sorted
 * 
 * @param Q The queue that INPUT should be placed in
 * @param INPUT A pointer to the data type to insert in the queue
 * @param NEXT_MEMBER the member of TYPE of this queue that represents the next
 * element in the queue.
 * @param VALUE_MEMBER the member of TYPE of this queue that represents the value
 * that the elements are sorted on
 * @param OP the comparison operator used to compare 2 adjacent elements
 */
#define QUEUE_SORTED_INSERT(Q, INPUT, NEXT_MEMBER, VALUE_MEMBER, OP) {           \
            if((INPUT)!=NULL) {                                                  \
                (Q).iterator = (Q).head;                                         \
                (Q).iterator_previous = NULL;                                    \
                while((Q).iterator != NULL) {                                    \
                    if((INPUT)->VALUE_MEMBER OP (Q).iterator->VALUE_MEMBER) {    \
                        break;                                                   \
                    }                                                            \
                    (Q).iterator_previous = (Q).iterator;                        \
                    (Q).iterator = (Q).iterator->NEXT_MEMBER;                    \
                }                                                                \
                if((Q).iterator_previous == NULL) {/*Item belongs at head*/      \
                    (INPUT)->NEXT_MEMBER = (Q).head;                             \
                    (Q).head = (INPUT);                                          \
                } else if((Q).iterator == NULL) {/*Item belongs at end*/         \
                    (INPUT)->NEXT_MEMBER = NULL;                                 \
                    (Q).iterator_previous->NEXT_MEMBER = (INPUT);                \
                } else {   /*Item belongs between 2 nodes*/                      \
                    (INPUT)->NEXT_MEMBER = (Q).iterator;                         \
                    (Q).iterator_previous->NEXT_MEMBER = (INPUT);                \
                }                                                                \
                (Q).count++;                                                     \
            }                                                                    \
    } while(0)

/**
 * @brief Wrapper for QUEUE_POP_FRONT_GENERIC
 * @details uses the default parameter 'next'
 * 
 * @param Q     An instance of a queue type (defined by CREATE_QUEUE_TYPE)
 * @param VALUE a TYPE pointer to store the resulting address that is popped from the queue
 */
#define QUEUE_POP_FRONT(Q, VALUE) {             \
    QUEUE_POP_FRONT_GENERIC(Q, VALUE, next);    \
} while(0)

/**
 * @brief Wrapper for QUEUE_PUSH_BACK_GENERIC
 * @details uses the default parameter 'next'
 * 
 * @param Q     An instance of a queue type (defined by CREATE_QUEUE_TYPE)
 * @param VALUE a TYPE pointer to store the resulting address that is popped from 
 */
#define QUEUE_PUSH_BACK(Q, VALUE) {             \
    QUEUE_PUSH_BACK_GENERIC(Q, VALUE, next);    \
} while(0)

#endif//__QUEUE_H__
