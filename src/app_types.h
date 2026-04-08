#ifndef APP_TYPES_H
#define APP_TYPES_H

typedef enum SortMode {
    SORT_BUBBLE,
    SORT_INSERTION,
    SORT_SELECTION,
    SORT_HEAP,
    SORT_QUICK,
    SORT_SHELL
} SortMode;

typedef enum DistributionMode {
    DIST_RANDOM,
    DIST_NEARLY_SORTED,
    DIST_REVERSED,
    DIST_FEW_UNIQUE,
    DIST_SAWTOOTH
} DistributionMode;

#endif
