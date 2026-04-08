#ifndef APP_TYPES_H
#define APP_TYPES_H

typedef enum SortMode {
    SORT_BUBBLE,
    SORT_INSERTION,
    SORT_SELECTION,
    SORT_HEAP,
    SORT_QUICK,
    SORT_SHELL,
    SORT_MERGE,
    SORT_COCKTAIL,
    SORT_GNOME,
    SORT_COMB,
    SORT_TIMSORT,
    SORT_RADIXSORT,
    SORT_BOGOSORT
} SortMode;

typedef enum DistributionMode {
    DIST_RANDOM,
    DIST_NEARLY_SORTED,
    DIST_REVERSED,
    DIST_FEW_UNIQUE,
    DIST_SAWTOOTH
} DistributionMode;

#endif
