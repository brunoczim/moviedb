#include "trie.h"
#include "alloc.h"

struct destroy_queue_node {
    struct trie_branch_list branches;
    struct destroy_queue_node *next;
};

struct destroy_queue {
    struct destroy_queue_node *front;
    struct destroy_queue_node *back;
};

static bool branch_search(
        struct trie_branch_list const *restrict list,
        char key,
        size_t *pos);

static void branch_insert(
        struct trie_branch_list *restrict branches,
        char key,
        struct trie_node *child,
        size_t branch_pos,
        struct error *error);

static struct trie_node *make_path(
        struct trie_node *node,
        size_t branch_pos,
        char const *restrict name,
        size_t *current_key,
        struct error *error);

static void destroy_queue_init(struct destroy_queue *restrict queue);

static void destroy_enqueue(
        struct destroy_queue *restrict queue,
        struct trie_branch_list const *branches,
        struct error *error);

static bool destroy_dequeue(
        struct destroy_queue *restrict queue,
        struct trie_branch_list *branches);

static inline bool destroy_queue_is_empty(struct destroy_queue *restrict queue);

static inline void destroy_branches_recursive(
        struct trie_branch_list *restrict branches);

static inline void destroy_enqueue_next_level(
        struct destroy_queue *restrict curr_level,
        struct destroy_queue *restrict next_level,
        struct error *error);

static inline void destroy_enqueue_children(
        struct destroy_queue *restrict queue,
        struct trie_branch_list *restrict branches,
        struct error *error);

static void destroy_recursive(struct trie_node *restrict root);

static inline void destroy_branch_list(struct trie_branch_list *branches);

extern inline void trie_root_init(struct trie_node *root);

void trie_insert(
        struct trie_node *root,
        char const *restrict name,
        long unsigned movieid,
        struct error *error)
{
    struct trie_node *node;
    size_t branch_pos;
    size_t current_key;
    bool found;

    current_key = 0;
    node = root;

    while (name[current_key] != 0 && error->code == error_none) {
        found = branch_search(&node->branches, name[current_key], &branch_pos);
        if (found) {
            node = node->branches.entries[branch_pos].child;
            current_key++;
        } else {
            node = make_path(node, branch_pos, name, &current_key, error);
        }
    }


    if (error->code == error_none) {
        node->has_leaf = true;
        node->movieid = movieid;
    }
}

bool trie_search(
        struct trie_node const *root,
        char const *restrict name,
        unsigned long *movieid_out)
{
    size_t current_key = 0;
    size_t branch_pos;
    struct trie_node const *node = root;
    bool branch_found = true;

    while (name[current_key] != 0 && branch_found) {
        branch_found = branch_search(
                &node->branches,
                name[current_key],
                &branch_pos);

        if (branch_found) {
            node = node->branches.entries[branch_pos].child;
            current_key++;
        }
    }

    if (branch_found && node->has_leaf && movieid_out != NULL) {
        *movieid_out = node->movieid;
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
        char const *restrict name,
        size_t *current_key,
        struct error *error)
{
    struct trie_node *child;
    
    while (name[*current_key] != 0 && error->code == error_none) {
        child = moviedb_alloc(sizeof(struct trie_node), error);
        if (error->code == error_none) {
            child->has_leaf = false;
            child->branches.entries = NULL;
            child->branches.length = 0;
            child->branches.capacity = 0;

            branch_insert(
                    &node->branches,
                    name[*current_key],
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
        struct trie_branch_list *branches)
{
    struct destroy_queue_node *next;

    if (queue->front == NULL) {
        return false;
    }

    *branches = queue->front->branches;
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

static void destroy_recursive(struct trie_node *restrict root)
{
    size_t i;

    for (i = 0; i < root->branches.length; i++) {
        destroy_recursive(root->branches.entries[i].child);
        moviedb_free(root->branches.entries);
        moviedb_free(root->branches.entries[i].child);
    }
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
