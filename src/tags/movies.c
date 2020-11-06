#include "movies.h"

/**
 * Pushes the given range to the stack. Only internal quicksort code is allowed
 * to touch this.
 */
static void sort_push(
        struct tag_movies_sort_stack *restrict stack,
        struct tag_movies_sort_range const *restrict range,
        struct error *restrict error);

/**
 * Tries to pop from the sort stack into range_out, returns whether the pop
 * happened. Only internal quicksort code is allowed to touch this.
 */
static bool sort_pop(
        struct tag_movies_sort_stack *restrict stack,
        struct tag_movies_sort_range *restrict range_out);

/**
 * Chooses the median as the pivot of an iteration of quicksort.
 */
static db_id_t sort_choose_median(
        struct tag_movie_list *restrict list,
        struct tag_movies_sort_range const *range);

/**
 * Performs the hoare partition on the given list, given the pivot.
 */
static size_t sort_partition_hoare(
        struct tag_movie_list *restrict list,
        db_id_t pivot,
        struct tag_movies_sort_range const *range);

bool tag_movies_contain(
        struct tag_movie_list const *restrict movies,
        db_id_t movieid)
{
    bool found;
    size_t low, high, mid;

    low = 0;
    high = movies->length;
    found = false;

    while (low < high && !found) {
        mid = low + (high - low) / 2;

        if (movies->entries[mid] < movieid) {
            low = mid + 1;
        } else if (movies->entries[mid] > movieid) {
            high = mid;
        } else {
            found = true;
        }
    }

    return found;
}

void tag_movies_sort_init(
        struct tag_movies_sort_stack *restrict stack,
        size_t capacity,
        struct error *error)
{
    stack->ranges = db_alloc(
            sizeof(struct tag_movies_sort_range) * capacity,
            error);
    stack->length = 0;
    stack->capacity = capacity;
}

extern inline void tag_movies_sort_destroy(
        struct tag_movies_sort_stack *restrict stack);

void tag_movies_sort(
        struct tag_movie_list *restrict list,
        struct tag_movies_sort_stack *restrict stack,
        struct error *restrict error)
{
    struct tag_movies_sort_range curr, lower, upper;
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

static void sort_push(
        struct tag_movies_sort_stack *restrict stack,
        struct tag_movies_sort_range const *restrict range,
        struct error *restrict error)
{
    struct tag_movies_sort_range *new_ranges;
    size_t new_cap;

    if (stack->length == stack->capacity) {
        new_cap = stack->capacity * 2;
        if (new_cap == 0) {
            new_cap = 1;
        }
        new_ranges = db_realloc(
                stack->ranges,
                sizeof(struct tag_movies_sort_range) * new_cap,
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
        struct tag_movies_sort_stack *restrict stack,
        struct tag_movies_sort_range *restrict range_out)
{
    if (stack->length == 0) {
        return false;
    }

    stack->length--;
    *range_out = stack->ranges[stack->length];
    return true;
}

static db_id_t sort_choose_median(
        struct tag_movie_list *restrict list,
        struct tag_movies_sort_range const *range)
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
        struct tag_movies_sort_range const *range)
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
