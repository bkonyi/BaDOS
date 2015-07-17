#include <trains/train_policy.h>
#include <trains/train_server.h>
/**
 * Features that we want for our policy
 * ii. 10cm past all branches on path finding
 * iii. Don't hit other fucking trains head on
 * iv. Trains can only occupy track they have obtained from the server.
 * v. The server never gives out pices of track that are already out.
 * vi. To avoid leapfrog deadlock, all the track owned by a train must be contiguous.
 * vii. Track should be returned to the track server as soon as a train leaves it.
 * 
 */

void none(){}
