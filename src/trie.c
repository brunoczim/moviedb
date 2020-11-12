#include "trie.h"
#include "alloc.h"

/**
 * Makes path for a node. Starts by creating a child in the given node, in the
 * given branch_pos (branch position), advancing current_key to the end of the
 * string. Returns a heap-allocated node where a moviedb_id_t with the given
 * title should be inserted as leaf. The only possible error is an allocation
 * error.
 */
static struct trie_node *make_path(
        struct trie_node *node,
        size_t branch_pos,
        char const *restrict title,
        size_t *restrict current_key,
        struct error *restrict error);


/**
 * Destroys the given branches recursively (last resort, in case
 * heap-allocation of the queue fails).
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

extern inline void trie_root_init(struct trie_node *restrict root);

void trie_insert(
        struct trie_node *root,
        char const *restrict title,
        moviedb_id_t movie,
        struct error *restrict error)
{
    struct trie_node *node;
    size_t branch_pos;
    size_t current_key;
    bool found;

    current_key = 0;
    node = root;

    /* Loops until the correct place of insertion is found. */
    while (title[current_key] != 0 && error->code == error_none) {
        found = trie_branches_search(
                &node->branches,
                title[current_key],
                &branch_pos);

        if (found) {
            /* Binary search found a place. */
            node = node->branches.entries[branch_pos].child;
            current_key++;
        } else {
            /*
             * In this case there won't be any nodes and we need to make a
             * path. Since we are passing current_key as a pointer, it will
             * change this cursor to a character which ends the string ('\0'),
             * and so, this will end the loop.
             */
            node = make_path(node, branch_pos, title, &current_key, error);
        }
    }


    if (error->code == error_none) {
        if (node->has_leaf) {
            /*
             * If there is already a leaf in this place, we have a duplicated
             * title. This can be ignored, though.
             */
            error_set_code(error, error_dup_movie_title);
            error->data.dup_movie_title.free_title = false;
            error->data.dup_movie_title.title = title;
        } else {
            /* Success case for the insertion. */
            node->has_leaf = true;
            node->movie = movie;
        }
    }
}

bool trie_search(
        struct trie_node const *root,
        char const *restrict title,
        moviedb_id_t *restrict movie_out)
{
    size_t current_key = 0;
    size_t branch_pos;
    struct trie_node const *node = root;
    bool branch_found = true;

    /*
     * Loops until we reached the right place of the title, or until we find out
     * there is no path to this title.
     */
    while (title[current_key] != 0 && branch_found) {
        branch_found = trie_branches_search(
                &node->branches,
                title[current_key],
                &branch_pos);

        if (branch_found) {
            /* Binary search found the branch position. */
            node = node->branches.entries[branch_pos].child;
            current_key++;
        }
    }

    if (branch_found && node->has_leaf && movie_out != NULL) {
        /*
         * If there is a path, a leaf at the end of the path, we found. Put
         * the movie ID in the output paramter.
         */
        *movie_out = node->movie;
    }

    return branch_found && node->has_leaf;
}

void trie_search_prefix(
        struct trie_node const *root,
        char const *restrict prefix,
        struct trie_iter *restrict iter_out)
{
    size_t current_key = 0;
    size_t branch_pos;
    struct trie_node const *node = root;
    bool branch_found = true;

    /*
     * Loops until we reached the right node, where the titles with the given
     * prefix are, or until we find out there is no path to titles with this
     * prefix.
     */
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

    /* Initializes the iterator. */
    trie_iter_queue_init(&iter_out->queue);
    iter_out->branch = 0;
    if (branch_found) {
        iter_out->current = node;
    } else {
        /* If we did not found simply make this iterator stop at the first
         * attempt.
         */
        iter_out->current = NULL;
    }
}

void trie_destroy(struct trie_node *root)
{
    /*
     * We'll be avoiding recursive destroy since it might result in stack
     * overflow. The trie could be large enough to do so.
     *
     * So, we will use the iterator queue to iterate over branch lists
     * iteratively. If any allocation on the iterator queue fails, we will use
     * recursive destroy as a fallback.
     */

    struct error error;
    /* Current level/depth in the trie. */
    struct trie_iter_queue curr_level;
    /* Next level/depth in the trie. */
    struct trie_iter_queue next_level;

    error_init(&error);

    trie_iter_queue_init(&curr_level);
    trie_iter_queue_init(&next_level);

    /* Initializes the current level queue with the root's branch list. */
    trie_iter_enqueue(&curr_level, &root->branches, &error);

    if (error.code == error_none) {
        while (!trie_iter_queue_is_empty(&curr_level)) {
            /*
             * Adds branch lists to the next level, based on the children of
             * the current level, and destroys current level.
             */
            destroy_enqueue_next_level(&curr_level, &next_level, &error);
            if (error.code == error_none) {
                /* Accounts that we are going to pass to the next level. */
                curr_level = next_level;
                trie_iter_queue_init(&next_level);
            }
        }
    } else {
        /* Uses a recursive destroy as a fallback if allocation error. */
        destroy_recursive(root);
    }
}

static struct trie_node *make_path(
        struct trie_node *node,
        size_t branch_pos,
        char const *restrict title,
        size_t *restrict current_key,
        struct error *restrict error)
{
    struct trie_node *child;
    
    /* Loops while we did not reach the end of the string (and no error). */
    while (title[*current_key] != 0 && error->code == error_none) {
        /* Allocates a child node. */
        child = moviedb_alloc(sizeof(struct trie_node), error);
        if (error->code == error_none) {
            child->has_leaf = false;
            child->branches.entries = NULL;
            child->branches.length = 0;
            child->branches.capacity = 0;

            /* Inserts the new child into the branch list. */
            trie_branches_insert(
                    &node->branches,
                    title[*current_key],
                    child,
                    branch_pos,
                    error);

            /* Prepares for the next iteration. */
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

    /*
     * Recursive destruction of the branch list, with deep destruction. Iterates
     * just at the top-level.
     */
    for (i = 0; i < branches->length; i++) {
        /* Delegates destruction of children to recursive function. */
        destroy_recursive(branches->entries[i].child);
        moviedb_free(branches->entries[i].child);
    }

    /* Just destroys the branch allocation. */
    trie_branches_destroy(branches);
}

static inline void destroy_enqueue_next_level(
        struct trie_iter_queue *restrict curr_level,
        struct trie_iter_queue *restrict next_level,
        struct error *restrict error)
{
    struct trie_branch_list branches;
    
    while (trie_iter_dequeue(curr_level, &branches)) {
        /* Eneuques the children of the current level into the next level. */
        trie_iter_enqueue_children(next_level, &branches, error);

        if (error->code == error_none) {
            /*
             * If no error, destroys the allocation of the branch list's array.
             */
            destroy_branch_list(&branches);
        } else {
            /* If an error happened, do emergency cleanup. */
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

    /*
     * Iterative destruction of the branch list, no deep destruction is
     * performed by this.
     */
    for (i = 0; i < branches->length; i++) {
        moviedb_free(branches->entries[i].child);
    }

    trie_branches_destroy(branches);
}


static void destroy_recursive(struct trie_node *restrict root)
{
    size_t i;

    /*
     * We are deallocating the root branches, but not the root itself, since it
     * can be stack-allocated.
     * */
    for (i = 0; i < root->branches.length; i++) {
        destroy_recursive(root->branches.entries[i].child);
        trie_branches_destroy(&root->branches.entries[i].child->branches);
        /*
         * Deallocates the child; it won't dellocate for itself, since it
         * thinks it is root when the recursive call happens.
         */
        moviedb_free(root->branches.entries[i].child);
    }
}
