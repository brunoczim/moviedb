#include "trie.h"
#include "alloc.h"

/**
 * A destroy queue's node.
 */
struct destroy_queue_node {
    /**
     * Branch to be destroyed.
     */
    struct trie_branch_list branches;
    /**
     * The next node in the queue.
     */
    struct destroy_queue_node *next;
};

/**
 * A destroy queue.
 */
struct destroy_queue {
    /**
     * Front of the queue (i.e. the output side).
     */
    struct destroy_queue_node *front;
    /**
     * Back of the queue (i.e. the input side).
     */
    struct destroy_queue_node *back;
};

/**
 * Searches for the given key in a branch list. Returns whether if the given key
 * was found. Fills pos with the position of the key. If not found, pos is
 * filled with the position where the key should be inserted.
 */
static bool branch_search(
        struct trie_branch_list const *restrict list,
        char key,
        size_t *pos);

/**
 * Inserts the given child into the given list of branches, using the given key.
 */
static void branch_insert(
        struct trie_branch_list *restrict branches,
        char key,
        struct trie_node *child,
        size_t branch_pos,
        struct error *error);

/**
 * Makes path for a node. Starts by creating a child in the given node, in the
 * given branch_pos (branch position), advancing current_key to the end of the
 * string. Returns a heap-allocated node where a moviedb_id with the given title 
 * should be inserted as leaf. The only possible error is an allocation error.
 */
static struct trie_node *make_path(
        struct trie_node *node,
        size_t branch_pos,
        char const *restrict title,
        size_t *current_key,
        struct error *error);

/**
 * Initializes the destroy queue.
 */
static void destroy_queue_init(struct destroy_queue *restrict queue);

/**
 * Enqueues a branch list on the destroy queue. The only possible error is an
 * allocation error.
 */
static void destroy_enqueue(
        struct destroy_queue *restrict queue,
        struct trie_branch_list const *branches,
        struct error *error);

/**
 * Dequeues a branch list from the destroy queue. Returns whether the dequeue
 * actually happened (i.e. the queue was not empty), and fills the branches_out
 * parameter with the queue if successful.
 */
static bool destroy_dequeue(
        struct destroy_queue *restrict queue,
        struct trie_branch_list *branches_out);

/**
 * Tests if the destroy queue is empty
 */
static inline bool destroy_queue_is_empty(struct destroy_queue *restrict queue);

/**
 * Destroys the given branches recursively (last resort, in case heap-allocation
 * of the queue fails).
 */
static inline void destroy_branches_recursive(
        struct trie_branch_list *restrict branches);

/**
 * Enqueues all children of all branches in the curr_level into the next_level.
 * The only possible error is an allocation error.
 */
static inline void destroy_enqueue_next_level(
        struct destroy_queue *restrict curr_level,
        struct destroy_queue *restrict next_level,
        struct error *error);

/**
 * Enqueues all children of of the given branch list into the given queue. The
 * only possible error is an allocation error.
 */
static inline void destroy_enqueue_children(
        struct destroy_queue *restrict queue,
        struct trie_branch_list *restrict branches,
        struct error *error);

/**
 * Definitely frees memory of the given branch list. Destroys also pointers to
 * children, **but not the children themselves**.
 */
static inline void destroy_branch_list(struct trie_branch_list *branches);

/**
 * Destroys a tree recursively. Should be a last resource, used only if
 * heap-allocation of the queue fails.
 */
static void destroy_recursive(struct trie_node *restrict root);

extern inline void trie_root_init(struct trie_node *root);

void trie_insert(
        struct trie_node *root,
        char const *restrict title,
        moviedb_id movie,
        struct error *error)
{
    struct trie_node *node;
    size_t branch_pos;
    size_t current_key;
    bool found;

    current_key = 0;
    node = root;

    while (title[current_key] != 0 && error->code == error_none) {
        found = branch_search(&node->branches, title[current_key], &branch_pos);
        if (found) {
            node = node->branches.entries[branch_pos].child;
            current_key++;
        } else {
            node = make_path(node, branch_pos, title, &current_key, error);
        }
    }


    if (error->code == error_none) {
        node->has_leaf = true;
        node->movie = movie;
    }
}

bool trie_search(
        struct trie_node const *root,
        char const *restrict title,
        moviedb_id *movie_out)
{
    size_t current_key = 0;
    size_t branch_pos;
    struct trie_node const *node = root;
    bool branch_found = true;

    while (title[current_key] != 0 && branch_found) {
        branch_found = branch_search(
                &node->branches,
                title[current_key],
                &branch_pos);

        if (branch_found) {
            node = node->branches.entries[branch_pos].child;
            current_key++;
        }
    }

    if (branch_found && node->has_leaf && movie_out != NULL) {
        *movie_out = node->movie;
    }

    return branch_found && node->has_leaf;
}

void trie_destroy(struct trie_node *root)
{
    struct error error;
    struct destroy_queue curr_level;
    struct destroy_queue next_level;

    error_init(&error);

    destroy_queue_init(&curr_level);
    destroy_queue_init(&next_level);

    destroy_enqueue(&curr_level, &root->branches, &error);

    if (error.code == error_none) {
        while (!destroy_queue_is_empty(&curr_level)) {
            destroy_enqueue_next_level(&curr_level, &next_level, &error);
            if (error.code == error_none) {
                curr_level = next_level;
                destroy_queue_init(&next_level);
            }
        }
    } else {
        destroy_recursive(root);
    }
}

static bool branch_search(
        struct trie_branch_list const *restrict list,
        char key,
        size_t *pos)
{
    size_t low, mid, high;
    bool found;

    found = false;
    low = 0;
    high = list->length;

    while (low < high && !found) {
        mid = low + (high - low) / 2;

        if (list->entries[mid].key < key) {
            low = mid + 1;
        } else if (list->entries[mid].key > key) {
            high = mid;
        } else {
            low = mid;
            found = true;
        }
    }

    *pos = low;

    return found;
}

static void branch_insert(
        struct trie_branch_list *restrict branches,
        char key,
        struct trie_node *child,
        size_t branch_pos,
        struct error *error)
{
    size_t i;
    size_t new_cap;
    struct trie_branch *new_entries;

    if (branches->length >= branches->capacity) {
        new_cap = branches->capacity * 2;
        if (new_cap == 0) {
            new_cap = 1;
        } else if (new_cap > 255) {
            new_cap = 255;
        }

        new_entries = moviedb_realloc(
                branches->entries,
                sizeof(struct trie_branch) * new_cap,
                error);
        if (error->code == error_none) {
            branches->entries = new_entries;
            branches->capacity = new_cap;
        }
    }

    if (error->code == error_none) {
        for (i = branches->length; i > branch_pos; i--) {
            branches->entries[i] = branches->entries[i - 1];
        }

        branches->entries[branch_pos].key = key;
        branches->entries[branch_pos].child = child;
        branches->length++;
    }
}

static struct trie_node *make_path(
        struct trie_node *node,
        size_t branch_pos,
        char const *restrict title,
        size_t *current_key,
        struct error *error)
{
    struct trie_node *child;
    
    while (title[*current_key] != 0 && error->code == error_none) {
        child = moviedb_alloc(sizeof(struct trie_node), error);
        if (error->code == error_none) {
            child->has_leaf = false;
            child->branches.entries = NULL;
            child->branches.length = 0;
            child->branches.capacity = 0;

            branch_insert(
                    &node->branches,
                    title[*current_key],
                    child,
                    branch_pos,
                    error);

            node = child;
            (*current_key)++;
            branch_pos = 0;
        }
    }

    return node;
}

static void destroy_queue_init(struct destroy_queue *restrict queue)
{
    queue->front = NULL;
    queue->back = NULL;
}

static void destroy_enqueue(
        struct destroy_queue *restrict queue,
        struct trie_branch_list const *branches,
        struct error *error)
{
    struct destroy_queue_node *node;

    node = moviedb_alloc(sizeof(struct destroy_queue_node), error);

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

static bool destroy_dequeue(
        struct destroy_queue *restrict queue,
        struct trie_branch_list *branches_out)
{
    struct destroy_queue_node *next;

    if (queue->front == NULL) {
        return false;
    }

    *branches_out = queue->front->branches;
    next = queue->front->next;
    moviedb_free(queue->front);
    queue->front = next;

    if (next == NULL) {
        queue->back = next;
    }

    return true;
}

static inline bool destroy_queue_is_empty(struct destroy_queue *restrict queue)
{
    return queue->front == NULL;
}

static inline void destroy_branches_recursive(
        struct trie_branch_list *restrict branches)
{
    size_t i;
    for (i = 0; i < branches->length; i++) {
        destroy_recursive(branches->entries[i].child);
        moviedb_free(branches->entries[i].child);
    }
    moviedb_free(branches->entries);
}

static inline void destroy_enqueue_next_level(
        struct destroy_queue *restrict curr_level,
        struct destroy_queue *restrict next_level,
        struct error *error)
{
    struct trie_branch_list branches;
    
    while (destroy_dequeue(curr_level, &branches)) {
        destroy_enqueue_children(next_level, &branches, error);
        if (error->code == error_none) {
            destroy_branch_list(&branches);
        } else {
            destroy_branches_recursive(&branches);
            while (destroy_dequeue(curr_level, &branches)) {
                destroy_branches_recursive(&branches);
            }
            while (destroy_dequeue(next_level, &branches)) {}
        }
    }
}

static inline void destroy_enqueue_children(
        struct destroy_queue *restrict queue,
        struct trie_branch_list *restrict branches,
        struct error *error)
{

    size_t i = 0;
    while (i < branches->length && error->code == error_none) {
        destroy_enqueue(
                queue,
                &branches->entries[i].child->branches,
                error);
        
        i++;
    }
}

static inline void destroy_branch_list(struct trie_branch_list *branches)
{
    size_t i;
    for (i = 0; i < branches->length; i++) {
        moviedb_free(branches->entries[i].child);
    }
    moviedb_free(branches->entries);
}


static void destroy_recursive(struct trie_node *restrict root)
{
    size_t i;

    for (i = 0; i < root->branches.length; i++) {
        destroy_recursive(root->branches.entries[i].child);
        moviedb_free(root->branches.entries);
        moviedb_free(root->branches.entries[i].child);
    }
}
