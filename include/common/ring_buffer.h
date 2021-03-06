#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#define IS_BUFFER_EMPTY(BUFFER) ((BUFFER).start == (BUFFER).end)
#define IS_BUFFER_FULL(BUFFER) ((((BUFFER).end + 1) % (BUFFER).size) == (BUFFER).start)
#define BUFFER_LENGTH(BUFFER) (((BUFFER).end < (BUFFER).start) ? ((BUFFER).end + (BUFFER).size) - (BUFFER).start : ((BUFFER).end - (BUFFER).start))

#define CREATE_RING_BUFFER_TYPE(NAME, TYPE, SIZE) \
    typedef struct {                \
        int size;                   \
        int start;                  \
        int end;                    \
        TYPE* buffer[(SIZE)];       \
    } NAME

#define CREATE_NON_POINTER_BUFFER_TYPE(NAME, TYPE, SIZE) \
    typedef struct {                \
        int size;                   \
        int start;                  \
        int end;                    \
        TYPE buffer[(SIZE)];        \
    } NAME

#define RING_BUFFER_INIT(BUFFER, SIZE) \
    (BUFFER).start = 0;                \
    (BUFFER).end   = 0;                \
    (BUFFER).size  = (SIZE)

#define POP_FRONT(BUFFER, VALUE) {                             \
        (VALUE) = (BUFFER).buffer[(BUFFER).start];             \
        (BUFFER).start = ((BUFFER).start + 1) % (BUFFER).size; \
        } while(0)

#define PUSH_BACK(BUFFER, INPUT, RESULT) {                 \
        (BUFFER).buffer[(BUFFER).end] = (INPUT);           \
        (BUFFER).end = ((BUFFER).end + 1) % (BUFFER).size; \
        if(IS_BUFFER_FULL(BUFFER)) {                       \
            (RESULT) = -1;                                 \
        }                                                  \
        (RESULT) = 0;                                      \
    } while(0)

#endif
