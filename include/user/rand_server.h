#ifndef __RAND_SERVER_H__
#define __RAND_SERVER_H__

/**
 * @brief Server responsible for returning psuedorandom numbers.
 * @details Server responsible for returning psuedorandom numbers. 
 * When sent a message, it will respond with a random 32-bit integer.
 */
void rand_server_task(void);

#endif
