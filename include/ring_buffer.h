#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#define IS_BUFFER_EMPTY(BUFFER) ((BUFFER)->start == (BUFFER)->end)
#define IS_BUFFER_FULL(BUFFER) ((((BUFFER)->end + 1) % (BUFFER)->size) == (BUFFER)->start)

#define CREATE_RING_BUFFER_TYPE(NAME, TYPE, SIZE) \
    typedef struct {                \
        int size;                   \
        int start;                  \
        int end;                    \
        TYPE* buffer[(SIZE)];       \
    } NAME;                         \
                                    \
    TYPE* pop_front(NAME* buf) {                        \
        TYPE* front = buf->buffer[buf->start];          \
        buf->start = (buf->start + 1) % buf->size;      \
        return front;                                   \
    }                                                   \
                                                        \
    int push_back(NAME* buf, TYPE* val) {               \
        buf->buffer[buf->end] = val;                    \
        buf->end = (buf->end + 1) % buf->size;          \
        if(IS_BUFFER_EMPTY(buf)) {                      \
            return -1;                                  \
        }                                               \
        return 0;                                       \
    }

#endif
