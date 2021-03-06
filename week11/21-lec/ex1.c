#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "./lib/xalloc.h"
#include "./lib/contracts.h"
#include "trie.h"

/******************************************************************************/
// Interface
/******************************************************************************/
char* string_clone(char* s);



/******************************************************************************/
// Implementation
/******************************************************************************/
typedef int compare_fn(elem e1, elem e2);
typedef void elem_free_fn(elem e);

struct buffer {
  int width;
  int height;
  int size;  // 0 <= size < width * height;
  elem buf;
  //  bool** mark;
  // (mark[i][j] == true) means board[i][j] has been added to buf
  //  compare* com;
};

typedef struct buffer* Buffer;

struct stack {
  int capacity;
  int top;
  elem* buf;
  compare_fn* cmp;
  elem_free_fn* elem_free;
};

typedef struct stack* Stack;

Stack stack_new(int size, compare_fn* cmp, elem_free_fn elem_free) {
  REQUIRES(size > 0);

  Stack s = (Stack) xcalloc(1, sizeof(struct stack));
  s->capacity = size;
  s->top = 0;
  s->buf = (elem*) xcalloc(size, sizeof(elem));
  s->cmp = cmp;
  s->elem_free = elem_free;
  return s;
}

void stack_free(Stack s) {
  if (s->elem_free != NULL) {
    for (int i = 0; i < s->top; i++) {
      (*s->elem_free)(s->buf[i]);
    }
  }
  free(s->buf);
  free(s);
  return;
}

bool stack_empty(Stack s) {
  return s->top == 0;
}

bool stack_full(Stack s) {
  return s->top == s->capacity;
}

void push(Stack s, elem e) {
  REQUIRES(!stack_full(s));

  s->buf[s->top++] = e;
}

elem pop(Stack s) {
  REQUIRES(!stack_empty(s));

  return s->buf[--s->top];
}

elem peek(Stack s) {
  REQUIRES(!stack_empty(s));

  return s->buf[s->top-1];
}

bool stack_contains(Stack s, elem e) {
  for (int i = 0; i < s->top; i++) {
    if (s->cmp(s->buf[i], e) == 0) {
      return true;
    }
  }
  return false;
}

/*
Buffer initialize_buffer(int width, int height) {
  Buffer b = (Buffer) xcalloc(1,sizeof(struct buffer));
  b->width = width;
  b->height = height;
  b->size = 0;
  b->buf = (char*) xcalloc(width*height+1, sizeof(char));
  //  b->mark = (bool**) xcalloc(width*height, sizeof(bool));
  return b;
}

void add_elem(Buffer buf, char c) {
  REQUIRES(buf->size < buf->width * buf->height - 1);

  buf->buf[buf->size++] = c;
}

char remove_elem(Buffer buf) {
  REQUIRES(buf->size > 0);

  buf->size--;
  buf->buf[buf->size] = 0;
}


struct board {
  char** cs; // chars
  int width;
  int height;
};

typedef board * Board;

Board board_new(char** chars, int width, int height) {
  Board b = (Board) xcalloc(struct board);
  b->cs = chars;
  b->width = width;
  b->height = height;
  return b;
}
*/

struct charPositionPair {
  int row;
  int col;
  char c;
};

typedef struct charPositionPair * CharPositionPair;

CharPositionPair charPositionPair_new(int row, int col, char c) {
  CharPositionPair pair =
    (CharPositionPair) xcalloc(1, sizeof(struct charPositionPair));
  pair->row = row;
  pair->col = col;
  pair->c = c;
  return pair;
}

int charPositionPair_compare(elem e1, elem e2) {
  //int charPositionPair_compare(CharPositionPair p1, CharPositionPair p2) {
  CharPositionPair p1 = (CharPositionPair) e1;
  CharPositionPair p2 = (CharPositionPair) e2;
  if (p1->row == p2->row && p1->col == p2->col) {
    return 0;
  } else if (p1->row < p2->row || (p1->row == p2->row && p1->col < p2->col)) {
    return -1;
  } else {
    return 1;
  }
}

void charPositionPair_free(elem e) {
  //  CharPositionPair pair = (CharPositionPair) e;
  free(e);
  return;
}
/******************************************************************************/

char* char_to_string(char c) {
  char* s = (char*) xcalloc(2, sizeof(char));
  sprintf(s, "%c", c);
  return s;
}

// RETURNS: a string that contains all chars in the char positions pairs in s
char* stack_to_string(Stack s) {
  char* str = (char*) xcalloc(s->capacity, sizeof(char));
  int i;
  for (i = 0; i < s->top; i++) {
    str[i] = ((CharPositionPair)s->buf[i])->c;
  }
  str[i] = '\0';
  return str;
}

void print_words_reachable(char** board, int width, int height,
                           trie dict, Stack s);

// GIVEN: a Boggle game board board[][] and a dictionary dict
// EFFECT: print all words in dict that contained in board
void BoggleGame(char** board, int width, int height, trie dict) {
  //  char* buf = (char*) xcalloc(width*height+1, sizeof(char));
  //  Buffer buf = initialize_buffer(width, height);

  Stack s = stack_new(width * height + 1, &charPositionPair_compare,
                      &charPositionPair_free);
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      char c = board[i][j];
      char* str = char_to_string(c);
      if (trie_prefix(dict, str)) {
        CharPositionPair pair = charPositionPair_new(i, j, board[i][j]);
        push(s, pair);
      // print_words(board, width,height, i, j, buf);
        print_words_reachable(board, width, height, dict, s);
        pop(s);
        free(pair);
      }
      free(str);
    }
  }
  stack_free(s);
  return;
}



// EFFECT: print all word in the baord that begin with chars in s
void print_words_reachable(char** board, int width, int height,
                           trie dict, Stack s) {
  CharPositionPair e = (CharPositionPair) peek(s);
  int row = e->row;
  int col = e->col;
  char* word = stack_to_string(s);
  if (trie_member(dict,word)) {
    printf("%s\n", word);
  }
  if (!trie_prefix(dict, word)) {
    return;
  }
  free(word);
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      if (0 <= row + i && row + i < height
          && 0 <= col + j && col + j < width) {
        CharPositionPair new_pair
          = charPositionPair_new(row+i, col+j, board[row+i][col+j]);
        if (stack_contains(s, new_pair) == false) {
          push(s, (elem)new_pair);
          print_words_reachable(board, width, height, dict, s);
          pop(s);
        }
        free((elem)new_pair);
      }
    }
  }
  return;
}


// RETURNS: an intialized dictionary
trie initialize_dictionary() {
  trie dict = trie_new();
  char* words[5]= {"hello", "world", "see", "bea", "beard"};
  int i = 0;
  for (i = 0; i < 5; i++) {
    trie_insert(dict, words[i]);
  }
  return dict;
}

char* string_clone(char* s) {
  int len = strlen(s);
  char* clone = (char*) xcalloc(len+1, sizeof(char));
  strncpy(clone, s, len+1);
  return clone;
}

// RETUNRS: an initialized Boggle game board of the given size
char** initialize_board() {
  char* tmp_board[] = {"efra", "hgdr", "psna", "eebe"};
  char** board = (char**) xcalloc(sizeof(char*)*5, sizeof(char));
  int i = 0;
  for (i = 0; i < 5; i++) {
    board[i] = string_clone(tmp_board[i]);
  }
  return board;
}

void free_board(char** board) {
  int len = sizeof(board);
  int i = 0;
  for (i = 0; i < len; i++) {
    free(board[i]);
  }
  return;
}

int main() {
  char** board = initialize_board();
  trie dict = initialize_dictionary();
  int width = 4;
  int height = 4;
  BoggleGame(board, width, height, dict);
  trie_free(dict);
  free_board(board);
  return 0;
}
