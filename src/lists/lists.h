

#pragma once

#define TIX_DYN_LIST(name, T)                                                  \
  typedef struct list_##name {                                                 \
    T **data;                                                                  \
    size_t cap;                                                                \
    size_t count;                                                              \
  } list_##name;                                                               \
  static inline int list_##name##_grow(list_##name *);                         \
  static inline int list_##name##_init(list_##name **l) {                      \
    *l = (list_##name *)malloc(sizeof(name));                                  \
    if (!l)                                                                    \
      return -1;                                                               \
    (*l)->data = (T **)malloc(64 * sizeof(T *));                               \
    (*l)->cap = 64;                                                            \
    (*l)->count = 0;                                                           \
    return 0;                                                                  \
  }                                                                            \
  static inline int list_##name##_grow(list_##name *l) {                       \
    size_t ns = l->cap * 2;                                                    \
    l->data = (T **)realloc(l->data, ns);                                      \
    l->cap = ns;                                                               \
    return 0;                                                                  \
  }                                                                            \
  static inline int list_##name##_add(list_##name *l, T *t) {                  \
    if (t == NULL)                                                             \
      return -1;                                                               \
    if (l->count >= l->cap)                                                    \
      list_##name##_grow(l);                                                   \
    l->data[l->count++] = (t);                                                 \
    return 0;                                                                  \
  }                                                                            \
  static inline T *list_##name##_get(list_##name *l, size_t i) {               \
    if (i >= l->count)                                                         \
      return NULL;                                                             \
    return l->data[i];                                                         \
  }
