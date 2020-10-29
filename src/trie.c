#include "trie.h"
#include "alloc.h"

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
        struct error *restrict error);


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
        struct trie_iter_queue *restrict curr_level,
        struct trie_iter_queue *restrict next_level,
        struct error *restrict error);

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
        struct error *restrict error)
{
    struct trie_node *node;
    size_t branch_pos;
    size_t current_key;
    bool found;

    current_key = 0;
    node = root;

    while (title[current_key] != 0 && error->code == error_none) {
        found = trie_branches_search(
                &node->branches,
                title[current_key],
                &branch_pos);

        if (found) {
            node = node->branches.entries[branch_pos].child;
            current_key++;
        } else {
            node = make_path(node, branch_pos, title, &current_key, error);
        }
    }


    if (error->code == error_none) {
        if (node->has_leaf) {
            error_set_code(error, error_dup_movie_title);
            error->data.dup_movie_title.free_title = false;
            error->data.dup_movie_title.title = title;
        } else {
            node->has_leaf = true;
            node->movie = movie;
        }
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
        branch_found = trie_branches_search(
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

void trie_search_prefix(
        struct trie_node const *root,
        char const *restrict prefix,
        struct trie_iter *iter_out)
{
    size_t current_key = 0;
    size_t branch_pos;
    struct trie_node const *node = root;
    bool branch_found = true;

    while (prefix[current_key] != 0 && branch_found) {
        branch_found = trie_branches_search(
                &node->branches,
                prefix[current_key],
                &branch_pos);

        if (branch_found) {
            node = node->branches.entries[branch_pos].child;
            current_key++;
        }
    }

    trie_iter_queue_init(&iter_out->queue);
    iter_out->branch = 0;
    if (branch_found) {
        iter_out->current = node;
    } else {
        iter_out->current = NULL;
    }
}

void trie_destroy(struct trie_node *root)
{
    struct error error;
    struct trie_iter_queue curr_level;
    struct trie_iter_queue next_level;

    error_init(&error);

    trie_iter_queue_init(&curr_level);
    trie_iter_queue_init(&next_level);

    trie_iter_enqueue(&curr_level, &root->branches, &error);

    if (error.code == error_none) {
        while (!trie_iter_queue_is_empty(&curr_level)) {
            destroy_enqueue_next_level(&curr_level, &next_level, &error);
            if (error.code == error_none) {
                curr_level = next_level;
                trie_iter_queue_init(&next_level);
            }
        }
    } else {
        destroy_recursive(root);
    }
}

static struct trie_node *make_path(
        struct trie_node *node,
        size_t branch_pos,
        char const *restrict title,
        size_t *current_key,
        struct error *restrict error)
{
    struct trie_node *child;
    
    while (title[*current_key] != 0 && error->code == error_none) {
        child = moviedb_alloc(sizeof(struct trie_node), error);
        if (error->code == error_none) {
            child->has_leaf = false;
            child->branches.entries = NULL;
            child->branches.length = 0;
            child->branches.capacity = 0;

            trie_branches_insert(
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

static inline void destroy_branches_recursive(
        struct trie_branch_list *restrict branches)
{
    size_t i;
    for (i = 0; i < branches->length; i++) {
        destroy_recursive(branches->entries[i].child);
        moviedb_free(branches->entries[i].child);
    }
    trie_branches_destroy(branches);
}

static inline void destroy_enqueue_next_level(
        struct trie_iter_queue *restrict curr_level,
        struct trie_iter_queue *restrict next_level,
        struct error *restrict error)
{
    struct trie_branch_list branches;
    
    while (trie_iter_dequeue(curr_level, &branches)) {
        trie_iter_enqueue_children(next_level, &branches, error);
        if (error->code == error_none) {
            destroy_branch_list(&branches);
        } else {
            destroy_branches_recursive(&branches);
            while (trie_iter_dequeue(curr_level, &branches)) {
                destroy_branches_recursive(&branches);
            }
            while (trie_iter_dequeue(next_level, &branches)) {}
        }
    }
}

static inline void destroy_branch_list(struct trie_branch_list *branches)
{
    size_t i;
    for (i = 0; i < branches->length; i++) {
        moviedb_free(branches->entries[i].child);
    }
    trie_branches_destroy(branches);
}


static void destroy_recursive(struct trie_node *restrict root)
{
    size_t i;

    for (i = 0; i < root->branches.length; i++) {
        destroy_recursive(root->branches.entries[i].child);
        trie_branches_destroy(&root->branches.entries[i].child->branches);
        moviedb_free(root->branches.entries[i].child);
    }
}
