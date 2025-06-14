#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "contiguous.h"

struct contiguous {
  struct cnode *first;
  void *upper_limit;
};

struct cnode {
  size_t nsize;
  struct cnode *prev;
  struct cnode *next;
  struct contiguous *block;
};

const int SIZEOF_CONTIGUOUS = sizeof(struct contiguous);
const int SIZEOF_CNODE = sizeof(struct cnode);



static const char STAR_STR[] = "*";
static const char NULL_STR[] = "NULL";

// maybe_null(void *p) return a pointer to "NULL" or "*",
//   indicating if p is NULL or not.
static const char *maybe_null(void *p) {
  return p ? STAR_STR : NULL_STR;
}

// gapsize(n0, n1) determine the size (in bytes) of the gap between n0 and n1.
static size_t gapsize(struct cnode *n0, struct cnode *n1) {
  assert(n0);
  assert(n1);
  void *v0 = n0;
  void *v1 = n1;
  return (v1 - v0) - n0->nsize - sizeof(struct cnode);
}

// print_gapsize(n0, n1) print the size of the gap between n0 and n1,
//     if it's non-zero.
static void print_gapsize(struct cnode *n0, struct cnode *n1) {
  assert(n0);
  assert(n1);
  size_t gap = gapsize(n0, n1);
  
  if (gap != 0) { 
    printf("%zd byte gap\n", gap);
  }
}


// pretty_print_block(chs, size) Print size bytes, starting at chs,
//    in a human-readable format: printable characters other than backslash
//    are printed directly; other characters are escaped as \xXX
static void pretty_print_block(unsigned char *chs, int size) {
  assert(chs);
  for (int i = 0; i < size; i++) {
    printf(0x20 <= chs[i] && chs[i] < 0x80 && chs[i] != '\\'
           ? "%c" : "\\x%02X", chs[i]);
  }
  printf("\n");
}

// print_node(node) Print the contents of node and all nodes that
//    follow it.  Return a pointer to the last node.
static struct cnode *print_node(struct cnode *node) {
  while (node != NULL) {
    void *raw = node + 1;     // point at raw data that follows.
    printf("struct cnode\n");
    printf("    nsize: %ld\n", node->nsize);
    printf("    prev: %s\n", maybe_null(node->prev));
    printf("    next: %s\n",  maybe_null(node->next));

    printf("%zd byte chunk: ", node->nsize);
    
    pretty_print_block(raw, node->nsize);
    
    if (node->next == NULL) {
      return node;
    } else {
      print_gapsize(node, node->next);
      node = node->next;
    }
  }
  return NULL;
}



static void print_hr(void) {
    printf("----------------------------------------------------------------\n");
}

// print_debug(block) print a long message showing the content of block.
void print_debug(struct contiguous *block) {
  assert(block);
  void *raw = block;

  print_hr();
  printf("struct contiguous\n");
  printf("    first: %s\n", maybe_null(block->first));

  if (block->first == NULL) {
    size_t gap = block->upper_limit - raw - sizeof(struct contiguous);
    printf("%zd byte gap\n", gap);           
  } else {
    void *block_first = block->first;
    size_t gap = block_first - raw - sizeof(struct contiguous);
    if (gap) {
      printf("%zd byte gap\n", gap);
    }
  }
 
  struct cnode *lastnode = print_node(block->first);
  
  if (lastnode != NULL) {
    print_gapsize(lastnode, block->upper_limit);
  }

  print_hr();
}



struct contiguous *make_contiguous(size_t size) {
  struct contiguous *block = malloc(size);
  if (block == NULL) {
    return NULL;
  }

  block->first = NULL;
  block->upper_limit = (void *)((char *)block + size);

  for (size_t i = sizeof(struct contiguous); i < size; i++) {
    *((char *)block + i) = '$';
  }

  return block;
}




void destroy_contiguous(struct contiguous *block) {
  assert(block);
  if (block->first != NULL) {
    printf("Destroying non-empty block!\n");
  }
  free(block);
}




void cfree(void *p) {
  if (!p) return;
  struct cnode* node = (struct cnode *)((char *)p - sizeof(struct cnode));
  if (node->prev) {
    node->prev->next = node->next;
  }
  else {
    node->block->first = node->next;
  }

  if (node->next) {
    node->next->prev = node->prev;
  }
}




void *cmalloc(struct contiguous *block, int size) {
  assert(block != NULL);
  assert(size >= 0);
  
  size_t total = sizeof(struct cnode) + size;
  assert(total > 0);
  
  struct cnode *curr = block->first;
  void *gap_loco = NULL;
  
  void *mem_start = (char *)block + sizeof(struct contiguous);
  size_t space = curr ? ((char *)curr - (char *)mem_start) 
                     : ((char *)block->upper_limit - (char *)mem_start);
  
  if (space >= total) {
      gap_loco = mem_start;
  }
  
  while (curr) {
      void *chunk_end = (char *)curr + sizeof(struct cnode) + curr->nsize;
      space = curr->next ? ((char *)curr->next - (char *)chunk_end) 
                        : ((char *)block->upper_limit - (char *)chunk_end);
      
      if (space >= total && (gap_loco == NULL || (char *)chunk_end < (char *)gap_loco)) {
          gap_loco = chunk_end;
      }
      
      curr = curr->next;
  }
  
  if (gap_loco == NULL) return NULL;
  
  struct cnode *new = (struct cnode *)gap_loco;
  new->nsize = size;
  new->block = block;
  new->prev = NULL;
  new->next = NULL;
  
  if (block->first == NULL || (char *)new < (char *)block->first) {
      new->next = block->first;
      if (block->first) {
          block->first->prev = new;
      }
      block->first = new;
  } else {
      struct cnode *after = block->first;
      while (after->next && (char *)after->next < (char *)new) {
          after = after->next;
      }
      
      new->prev = after;
      new->next = after->next;
      
      if (after->next) {
          after->next->prev = new;
      }
      after->next = new;
  }
  
  return (char *)new + sizeof(struct cnode);
}

