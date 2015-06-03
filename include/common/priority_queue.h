#ifndef _PRIORITY_QUEUE_H_
#define _PRIORITY_QUEUE_H_
#include <global.h>



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
#define CREATE_PRIORITY_QUEUE_TYPE(NAME, TYPE)           \
    typedef struct {                            \
        TYPE* head;                             \
        TYPE* iterator_previous;				\
        TYPE* iterator;							\
        uint32_t count;                         \
    } NAME;

#define PRIORITY_QUEUE_INIT(Q) {				\
    	Q.head = NULL;							\
    	Q.iterator_previous = NULL;				\
    	Q.iterator = NULL;						\
    	Q.count = 0;							\
	} while(0)

/**
 * @brief removes the head of the queue and stores it's pointer in VALUE
 * 
 * @param Q     An instance of a queue type (defined by CREATE_QUEUE_TYPE)
 * @param VALUE a TYPE pointer to store the resulting address that is popped from the queue
 */
#define PRIORITY_QUEUE_POP_NEXT(Q, OUTPUT_VALUE, NEXT_MEMBER) {     \
        if(Q.count == 0) OUTPUT_VALUE = NULL;                  		\
        else{     													\
            OUTPUT_VALUE = Q.head;                             		\
            Q.head = Q.head->NEXT_MEMBER;               			\
            Q.count--;                                  			\
        }                                 							\
    } while(0)


/**
 * @brief pushes a TYPE pointer onto the back of a queue object
 * 
 * @param Q     An instance of a queue type (defined by CREATE_QUEUE_TYPE)
 * @param INPUT the TYPE pointer to push onto the end of Q
 */
#define PRIORITY_QUEUE_INSERT(Q, INPUT, NEXT_MEMBER, VALUE_MEMBER) {           \
            if(INPUT!=NULL) {                                                  \
            	(Q).iterator = (Q).head;                                           \
            	(Q).iterator_previous = NULL;                                    \
                while((Q).iterator != NULL) {                                    \
	        		if(INPUT->VALUE_MEMBER <= (Q).iterator->VALUE_MEMBER) {      \
	        			break;                                                 \
	        		}                                                          \
	        		(Q).iterator++;                                              \
	        	}                                                              \
                if((Q).iterator_previous == NULL) {                              \
                	INPUT->NEXT_MEMBER = (Q).head;                               \
                	(Q).head = INPUT;                                            \
                }else if((Q).iterator == NULL) {                                 \
                	(Q).iterator_previous->NEXT_MEMBER = INPUT;                  \
                }else {                                                        \
                	INPUT->NEXT_MEMBER = (Q).iterator;                           \
                	(Q).iterator_previous->NEXT_MEMBER = INPUT;                  \
                }                                                              \
                (Q).count++;                                                     \
            }                                                                  \
    } while(0)





#endif // _PRIORITY_QUEUE_H_
