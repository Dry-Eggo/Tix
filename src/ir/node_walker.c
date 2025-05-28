#include "ir_gen.h"
#include <stdio.h>
#include <stdlib.h>

void Node_init(Node_Walker *n, Program *p) {
  n->program = p;
  n->list = (IR_list *)malloc(sizeof(IR_list));
  n->list->insts = (TIR_Instruction **)malloc(64 * 8);
  n->list->cap = 64;
  n->list->count = 0;
  n->max = p->count;
  n->pos = 0;
}
Node *Node_next(Node_Walker *n) {
  if (n->pos >= n->max)
    return NULL;
  return n->program->nodes[n->pos++];
}

void Node_parse_fn(Item *item) {
  const char *function_name = item->fn.name;
  const char *function_rettype = Type_toraw(item->fn.return_type);
  ir_write("\nfunction (%s) %%%s:", function_rettype, function_name);
  ir_write("\nend");
}
void Node_add_inst(Node_Walker *n, TIR_Instruction *i) {
  IR_list_add(n->list, i);
}
void Node_parse_item(Item *item) {
  switch (item->kind) {
  case ITEM_FN: {
    Node_parse_fn(item);
  } break;
  default:
    printf("Invalid Item\n");
    exit(1);
  }
}

void Node_generate(Node_Walker *n) {
  while (n->pos < n->max) {
    Node *node = Node_next(n);
    switch (node->type) {
    case TNODE_ITEM: {
      Node_parse_item(node->node.item);
    } break;
    default:
      printf("Invalid Ast\n");
      exit(1);
    }
  }
}
