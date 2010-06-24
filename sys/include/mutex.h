#ifndef MUTEX_H
#define MUTEX_H

typedef unsigned int mutex_t;

int test_and_set (int new_value, mutex_t *lock_pointer);

#endif
