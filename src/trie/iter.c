#include "iter.h"
#include "../alloc.h"
#include "../trie.h"

bool trie_next_movie(
    struct trie_iter *restrict iter,
    moviedb_id *restrict movie_out,
    struct error *restrict error)
{
    bool leaf = false;

    while (!leaf && iter->current != NULL && error->code == error_none) {
        if (iter->current->has_leaf) {
            *movie_out = iter->current->movie;
            leaf = true;
        }

        trie_next(iter, error);
    }

    return leaf;
}

void trie_iter_destroy(struct trie_iter *restrict iter)
{
    while (trie_iter_dequeue(&iter->queue, NULL)) {}
}

extern inline void trie_iter_queue_init(struct trie_iter_queue *restrict queue);

void trie_iter_enqueue(
        struct trie_iter_queue *restrict queue,
        struct trie_branch_list const *restrict branches,
        struct error *restrict error)
{
    struct trie_iter_node *node;

    node = moviedb_alloc(sizeof(struct trie_iter_node), error);

    if (error->code == error_none) {
        node->next = NULL;
        node->branches = *branches;

        if (queue->back == NULL) {
            queue->front = node;
        } else {
            queue->back->next = node;
        }
        queue->back = node;
    }
}

bool trie_iter_dequeue(
        struct trie_iter_queue *restrict queue,
        struct trie_branch_list *restrict branches_out)
{
    struct trie_iter_node *next;

    if (queue->front == NULL) {
        return false;
    }

    if (branches_out != NULL) {
        *branches_out = queue->front->branches;
    }
    next = queue->front->next;
    moviedb_free(queue->front);
    queue->front = next;

    if (next == NULL) {
        queue->back = next;
    }

    return true;
}

extern inline bool trie_iter_queue_is_empty(
        struct trie_iter_queue const *restrict queue);

void trie_next(struct trie_iter *restrict iter, struct error *restrict error)
{
    bool out_of_bounds;

    if (iter->current != NULL) {
        trie_iter_enqueue_children(
                &iter->queue,
                &iter->current->branches,
                error);
    }

    do {
        out_of_bounds = false;

        if (!trie_iter_queue_is_empty(&iter->queue)) {
            if (iter->branch >= iter->queue.front->branches.length) {
                out_of_bounds = true;
                trie_iter_dequeue(&iter->queue, NULL);
                iter->branch = 0;
            }
        }
    } while (out_of_bounds);

    if (trie_iter_queue_is_empty(&iter->queue)) {
        iter->current = NULL;
    } else {
        iter->current = iter->queue.front->branches.entries[iter->branch].child;
        iter->branch++;
    }
}

void trie_iter_enqueue_children(
        struct trie_iter_queue *restrict queue,
        struct trie_branch_list const *restrict branches,
        struct error *restrict error)
{

    size_t i = 0;
    while (i < branches->length && error->code == error_none) {
        trie_iter_enqueue(
                queue,
                &branches->entries[i].child->branches,
                error);
        
        i++;
    }
}
