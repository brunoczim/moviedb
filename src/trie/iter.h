#ifndef MOVIEDB_TRIE_ITER_H
#define MOVIEDB_TRIE_ITER_H 1

#include "branch.h"
#include "../id.h"

/**
 * This file defines items related to branches of a trie. Some items are
 * private. 
 */

/**
 * An iterator queue's node. Only trie internal code is allowed to touch this.
 */
struct trie_iter_node {
    /**
     * Branch list currently being iterated. Only trie internal code is allowed
     * to touch this.
     */
    struct trie_branch_list branches;
    /**
     * The next node in the queue. Only trie internal code is allowed to touch
     * this.
     */
    struct trie_iter_node *next;
};

/**
 * An iterator queue. Only trie internal code is allowed to touch this.
 */
struct trie_iter_queue {
    /**
     * Front of the queue (i.e. the output side). Only trie internal code is
     * allowed to touch this.
     */
    struct trie_iter_node *front;
    /**
     * Back of the queue (i.e. the input side). Only trie internal code is
     * allowed to touch this.
     */
    struct trie_iter_node *back;
};

/**
 * Iterator over a trie's node and the children.
 */
struct trie_iter {
    /**
     * The queue used to store the children of iterated nodes. Do not touch this
     * field.
     */
    struct trie_iter_queue queue;
    /**
     * Index of the current branch. Only trie internal code is allowed to touch
     * this.
     */
    size_t branch;
    /**
     * The current value produced by the iterator. Only trie internal code is
     * allowed to touch this.
     */
    struct trie_node const *current;
};

/**
 * Advances the iterator and puts the current movie ID in the out paramter
 * movie_out.
 */
bool trie_next_movie(
    struct trie_iter *restrict iter,
    moviedb_id *restrict movie_out,
    struct error *restrict error);

/**
 * Destroy resources used by the trie iterator.
 */
void trie_iter_destroy(struct trie_iter *restrict iter);

/**
 * Initializes the iteration queue.
 */
inline void trie_iter_queue_init(struct trie_iter_queue *restrict queue)
{
    queue->front = NULL;
    queue->back = NULL;
}

/**
 * Enqueues a branch list on the iteration queue. The only possible error is an
 * allocation error. Only trie internal code is allowed to touch this.
 */
void trie_iter_enqueue(
        struct trie_iter_queue *restrict queue,
        struct trie_branch_list const *restrict branches,
        struct error *restrict error);

/**
 * Dequeues a branch list from the iteration queue. Returns whether the dequeue
 * actually happened (i.e. the queue was not empty), and fills the branches_out
 * parameter with the queue if successful. Only trie internal code is allowed
 * to touch this.
 */
bool trie_iter_dequeue(
        struct trie_iter_queue *restrict queue,
        struct trie_branch_list *restrict branches_out);

/**
 * Tests if the iteration queue is empty. Only trie internal code is allowed
 * to touch this.
 */
inline bool trie_iter_queue_is_empty(
        struct trie_iter_queue const *restrict queue)
{
    return queue->front == NULL;
}

/**
 * Advances the iterator. Only allocation errors are possible. Only trie
 * internal code is allowed to touch this.
 */
void trie_next(struct trie_iter *restrict iter, struct error *restrict error);

/**
 * Enqueues all children of of the given branch list into the given queue. The
 * only possible error is an allocation error. Only trie internal code is
 * allowed to touch this.
 */
void trie_iter_enqueue_children(
        struct trie_iter_queue *restrict queue,
        struct trie_branch_list const *restrict branches,
        struct error *restrict error);

#endif
