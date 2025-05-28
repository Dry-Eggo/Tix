#include "node.h"
#include <stdlib.h>

int program_init(Program *p) {
  p->nodes = (Node **)malloc(64 * 8);
  if (!p->nodes)
    return -1;
  p->cap = 64;
  p->count = 0;
  return 0;
}
int program_add(Program *p, Node *n) {
  if (p->count >= p->cap)
    program_grow(p);
  p->nodes[p->count++] = n;
  return 0;
}
int program_grow(Program *p) {
  p->nodes = (Node **)realloc(p->nodes, p->cap * 2);
  p->cap *= 2;
  return 0;
}

Node *create_nodei(Item *i) {
  Node *node = (Node *)malloc(sizeof(Node));
  node->node.item = i;
  node->type = TNODE_ITEM;
  return node;
}
