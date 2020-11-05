#include "tags.h"
#include "prime.h"
#include "alloc.h"
#include <string.h>

#define MAX_LOAD 0.5

/**
 * Range of a list in a quicksort implementation. Only quicksort internal code
 * is allowed to touch this.
 */
struct sort_range {
    /**
     * Start index, inclusive.
     */
    size_t start;
    /**
     * End index, exclusive.
     */
    size_t end;
};

/**
 * Stack used by the quicksort-related functions.
 */
struct sort_stack {
    /**
     * The elements of the stack. Only quicksort internal code is allowed to
     * touch this.
     */
    struct sort_range *ranges;
    /**
     * Number of elements in the stack. Only quicksort internal code is allowed
     * to touch this.
     */
    size_t length;
    /**
     * How many elements the stack can store. Only quicksort internal code is
     * allowed to * touch this.
     */
    size_t capacity;
};

/**
 * Initalizes the stack with the given capacity.
 */
static void sort_init(
        struct sort_stack *restrict stack,
        size_t capacity,
        struct error *error);

/**
 * Pushes the given range to the stack. Only internal quicksort code is allowed
 * to touch this.
 */
static void sort_push(
        struct sort_stack *restrict stack,
        struct sort_range const *restrict range,
        struct error *restrict error);

/**
 * Tries to pop from the sort stack into range_out, returns whether the pop
 * happened. Only internal quicksort code is allowed to touch this.
 */
static bool sort_pop(
        struct sort_stack *restrict stack,
        struct sort_range *restrict range_out);

/**
 * Destroys the sort stack.
 */
static void sort_destroy(struct sort_stack *restrict stack);

/**
 * Chooses the median as the pivot of an iteration of quicksort.
 */
static db_id_t sort_choose_median(
        struct tag_movie_list *restrict list,
        struct sort_range const *range);

/**
 * Performs the hoare partition on the given list, given the pivot.
 */
static size_t sort_partition_hoare(
        struct tag_movie_list *restrict list,
        db_id_t pivot,
        struct sort_range const *range);

/**
 * Performs quicksort on the given movie list.
 */
static void quicksort(
        struct tag_movie_list *restrict list,
        struct sort_stack *restrict stack,
        struct error *restrict error);

/**
 * Initializes a tag by allocating a tag in the heap.
 */
static struct tag *tag_init(
        struct tag_csv_row *restrict tag_row,
        struct error *restrict error);

/**
 * Inserts a movie in the given tag movie list.
 */
static void tag_movies_insert(
        struct tag_movie_list *restrict movies,
        db_id_t movie,
        struct error *restrict error);

/**
 * Probes the given table until the place where the given tag name should be
 * stored, given its hash. Returns the index of this place.
 */
static size_t probe_index(
        struct tags_table const *restrict table,
        char const *restrict name,
        db_hash_t hash);

/**
 * Resizes the table to have at least double capacity.
 */
static void resize(
        struct tags_table *restrict table,
        struct error *restrict error);

void tags_init(
        struct tags_table *restrict table,
        size_t initial_capacity,
        struct error *restrict error)
{
    size_t i;

    table->length = 0;
    table->capacity = next_prime(initial_capacity);
    table->entries = db_alloc(
            sizeof(struct tag **) * table->capacity,
            error);

    if (error->code == error_none) {
        for (i = 0; i < table->capacity; i++) {
            table->entries[i] = NULL;
        }
    }
}

void tags_insert(
        struct tags_table *restrict table,
        struct tag_csv_row *restrict tag_row,
        struct error *restrict error)
{
    double load;
    db_hash_t hash;
    size_t index;

    load = (table->length + 1) / (double) table->capacity;
    if (load >= MAX_LOAD) {
        resize(table, error);
    }

    if (error->code == error_none) {
        hash = db_hash_str(tag_row->name);
        index = probe_index(table, tag_row->name, hash);

        if (table->entries[index] == NULL) {
            table->entries[index] = tag_init(tag_row, error);
            if (error->code == error_none) {
                table->length++;
            }
        } else {
            tag_movies_insert(
                    &table->entries[index]->movies,
                    tag_row->movieid,
                    error);

            tag_row_destroy(tag_row);
        }
    }
}

struct tag const *tags_search(
        struct tags_table const *restrict table,
        char const *restrict name)
{
    db_hash_t hash = db_hash_str(name);
    size_t index = probe_index(table, name, hash);
    return table->entries[index];
}

void tags_sort_movies(
        struct tags_table *restrict table,
        struct error *restrict error)
{
    struct sort_stack stack;
    size_t i = 0;

    sort_init(&stack, 2, error);

    if (error->code == error_none) {
        while (i < table->capacity && error->code == error_none) {
            if (table->entries[i] != NULL) {
                quicksort(&table->entries[i]->movies, &stack, error);
            }
            i++;
        }
        sort_destroy(&stack);
    }
}

void tags_destroy(struct tags_table *restrict table)
{
    size_t i;

    for (i = 0; i < table->capacity; i++) {
        if (table->entries[i] != NULL) {
            db_free(table->entries[i]->movies.entries);
            db_free((void *) (void const *) table->entries[i]->name);
            db_free(table->entries[i]);
        }
    }

    db_free(table->entries);
}

static void sort_init(
        struct sort_stack *restrict stack,
        size_t capacity,
        struct error *error)
{
    stack->ranges = db_alloc(sizeof(struct sort_range) * capacity, error);
    stack->length = 0;
    stack->capacity = capacity;
}

static void sort_push(
        struct sort_stack *restrict stack,
        struct sort_range const *restrict range,
        struct error *restrict error)
{
    struct sort_range *new_ranges;
    size_t new_cap;

    if (stack->length == stack->capacity) {
        new_cap = stack->capacity * 2;
        if (new_cap == 0) {
            new_cap = 1;
        }
        new_ranges = db_realloc(
                stack->ranges,
                sizeof(struct sort_range) * new_cap,
                error);

        if (error->code == error_none) {
            stack->capacity = new_cap;
            stack->ranges = new_ranges;
        }
    }

    stack->ranges[stack->length] = *range;
    stack->length++;
}

static bool sort_pop(
        struct sort_stack *restrict stack,
        struct sort_range *restrict range_out)
{
    if (stack->length == 0) {
        return false;
    }
    stack->length--;
    *range_out = stack->ranges[stack->length];
    return true;
}

static void sort_destroy(struct sort_stack *restrict stack)
{
    db_free(stack->ranges);
}

static db_id_t sort_choose_median(
        struct tag_movie_list *restrict list,
        struct sort_range const *range)
{
    db_id_t tmp;
    size_t half = range->start + (range->end - range->start) / 2;

    if (list->entries[half] > list->entries[range->end - 1]) {
        tmp = list->entries[half];
        list->entries[half] = list->entries[range->end - 1];
        list->entries[range->end - 1] = tmp;
    }

    if (list->entries[half] < list->entries[range->start]) {
        tmp = list->entries[half];
        list->entries[half] = list->entries[range->start];
        list->entries[range->start] = tmp;

        if (list->entries[half] > list->entries[range->end - 1]) {
            tmp = list->entries[half];
            list->entries[half] = list->entries[range->end - 1];
            list->entries[range->end - 1] = tmp;
        }
    }

    return list->entries[half];
}

static size_t sort_partition_hoare(
        struct tag_movie_list *restrict list,
        db_id_t pivot,
        struct sort_range const *range)
{
    db_id_t tmp;
    size_t low = range->start;
    size_t high = range->end - 1;
    bool elems_left = true;

    while (elems_left) {
        do {
            low++;
        } while (list->entries[low] < pivot);

        do {
            high--;
        } while (list->entries[high] > pivot);

        elems_left = low < high;

        if (elems_left) {
            tmp = list->entries[low];
            list->entries[low] = list->entries[high];
            list->entries[high] = tmp;
        }
    }

    return high + 1;
}

static void quicksort(
        struct tag_movie_list *restrict list,
        struct sort_stack *restrict stack,
        struct error *restrict error)
{
    struct sort_range curr, lower, upper;
    size_t split;
    db_id_t pivot;

    curr.start = 0;
    curr.end = list->length;

    if (curr.end - curr.start > 1) {
        sort_push(stack, &curr, error);
    }

    while (error->code == error_none && sort_pop(stack, &curr)) {
        pivot = sort_choose_median(list, &curr);
        split = sort_partition_hoare(list, pivot, &curr);

        lower.start = curr.start;
        lower.end = split;

        if (lower.end - lower.start > 1) {
            sort_push(stack, &lower, error);
        }

        upper.start = split;
        upper.end = curr.end;

        if (upper.end - upper.start > 1) {
            sort_push(stack, &upper, error);
        }
    }
}

static struct tag *tag_init(
        struct tag_csv_row *restrict tag_row,
        struct error *restrict error)
{
    struct tag *tag = db_alloc(sizeof(struct tag), error);

    if (error->code == error_none) {
        tag->name = tag_row->name;
        tag->movies.length = 1;
        tag->movies.capacity = 1;
        tag->movies.entries = db_alloc(
                sizeof(struct tag) * tag->movies.capacity,
                error);

        if (error->code == error_none) {
            tag->movies.entries[0] = tag_row->movieid;
        } else {
            db_free(tag);
            tag = NULL;
        }
    }

    return tag;
}

static void tag_movies_insert(
        struct tag_movie_list *restrict movies,
        db_id_t movie,
        struct error *restrict error)
{
    db_id_t *new_entries;
    size_t new_cap = movies->capacity * 2;

    if (movies->length == movies->capacity) {
        new_entries = db_realloc(
                movies->entries,
                sizeof(db_id_t) * new_cap,
                error);

        if (error->code == error_none) {
            movies->entries = new_entries;
            movies->capacity = new_cap;
        }
    }

    if (error->code == error_none) {
        movies->entries[movies->length] = movie;
        movies->length++;
    }
}

static size_t probe_index(
        struct tags_table const *restrict table,
        char const *restrict name,
        db_hash_t hash)
{
    db_hash_t attempt;
    size_t index;
    struct tag *tag;

    attempt = 0;
    index = db_hash_to_index(hash, attempt, table->capacity);
    tag = table->entries[index];

    while (tag != NULL && strcmp(tag->name, name) != 0) {
        attempt++;
        index = db_hash_to_index(hash, attempt, table->capacity);
        tag = table->entries[index];
    }

    return index;
}

static void resize(
        struct tags_table *restrict table,
        struct error *restrict error)
{
    size_t i;
    db_hash_t hash;
    size_t index;
    struct tags_table new_table;

    if (SIZE_MAX / 2 >= table->capacity) {
        new_table.capacity = next_prime(table->capacity * 2);
    } else {
        new_table.capacity = SIZE_MAX;
    }

    new_table.entries = db_alloc(
            sizeof(struct tag **) * new_table.capacity,
            error);

    if (error->code == error_none) {
        for (i = 0; i < new_table.capacity; i++) {
            new_table.entries[i] = NULL;
        }

        for (i = 0; i < table->capacity; i++) {
            if (table->entries[i] != NULL) {
                hash = db_hash_str(table->entries[i]->name);
                index = probe_index(&new_table, table->entries[i]->name, hash);
                new_table.entries[index] = table->entries[i];
            }
        }

        db_free(table->entries);
        table->capacity = new_table.capacity;
        table->entries = new_table.entries;
    }
}
