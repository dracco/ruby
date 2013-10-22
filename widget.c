/**********************************************************************

  array.c -

  $Author$
  created at: Fri Aug  6 09:46:12 JST 1993

  Copyright (C) 1993-2007 Yukihiro Matsumoto
  Copyright (C) 2000  Network Applied Communication Laboratory, Inc.
  Copyright (C) 2000  Information-technology Promotion Agency, Japan

**********************************************************************/

#include "ruby/ruby.h"
#include "ruby/util.h"
#include "ruby/st.h"
#include "ruby/encoding.h"
#include "internal.h"

VALUE rb_cWidget;

#define RWIDGET(obj) (R_CAST(RWidget)(obj))
#define RWIDGET_LEN(string) RWIDGET(string)->len
typedef struct {
    long start;
    long end;
    long queue[100000];
} queue_t;

int empty(queue_t *queue)
{
    return queue->start - queue->end;
}

int any(queue_t *queue)
{
    return queue->start != queue->end;
}


long dequeue(queue_t *queue)
{
    return queue->queue[queue->start++];
}

void enqueue(queue_t *queue, long value)
{
    queue->queue[queue->end++] = value;
}

long* zeros(long n)
{
  return ((long*)xmalloc2((n*n*n*n),sizeof(long)));
  /* return ALLOC(LONG2NUM)(long*)malloc(n*n*n*n*sizeof(long)); */
}

void grid_graph(long n, long graph[])
{
  long i;
  long size = n * n;
  long total = size * size;

  for (i = 1; i < size; i++)
  {
    long col = i;
    long row = i - 1;
    if ((row + 1) % n != 0) {
      if (row < size && col < size) {
        graph[row * size + col] = 1;
        graph[col * size + row] = 1;
      }
    }
  }
  for (i = n; i < size; i++)
  {
    long col = i;
    long row = i - n;
    long idx = row * size + col;
    if (row < size && col < size) {
        graph[row * size + col] = 1;
        graph[col * size + row] = 1;
    }
  }
}

typedef struct {
  long *list;
  long tail;
} list_t;

void list_append(list_t *list, long value)
{
  list->list[++list->tail] = value;
}

void bfs99(long n, long *graph, long *path_, long source)
{
  long size = n * n;
  list_t path = {path_, -1};
  queue_t queue = {0, 0, {0}};
  long visited[100000] = {0};

  enqueue(&queue, source);
  visited[source] = 1;
  while(any(&queue))
  {
    long node = dequeue(&queue);
    list_append(&path, node);
    long row_start = node * (size);
    int i;
    for (i = 0; i < size; i++)
    {
      if (graph[row_start + i] == 1)
      {
        if (visited[i] == 0) {
          enqueue(&queue, i);
          visited[i] = 1;
        }
      }
    }
  }
}






static VALUE
rb_wgt_inspect(VALUE ary)
{
  long len;
  len = RWIDGET(ary)->as.heap.len;
  fprintf(stderr, "*** rb_wgt_inspect");
  fprintf(stderr, "*** len: %lu", len);
  return rb_usascii_str_new2("[]");
}

static VALUE
rb_wgt_init(int argc, VALUE *argv, VALUE ary)
{
    if (argc == 1) {
      RWIDGET(ary)->as.heap.len = NUM2LONG(argv[0]);
      long size = NUM2LONG(argv[0]);
      long *graph = zeros(size);

      grid_graph(size, graph);
      RWIDGET(ary)->as.heap.graph = graph;
    };
    return ary;
}

static inline VALUE
wgt_alloc(VALUE klass)
{
    NEWOBJ(wgt, struct RWidget);
    OBJSETUP(wgt, klass, T_WIDGET);

    return (VALUE)wgt;
}

#define ARY_MAX_SIZE (LONG_MAX / (int)sizeof(VALUE))

long alen(VALUE ary)
{
    return RARRAY_LEN(ary);
}

long n2l(VALUE val)
{
    return NUM2LONG(val);
}

long ae(VALUE ary, long index)
{
    return rb_ary_entry(ary, index);
}

static
VALUE
rb_wgt_bfs_adj(VALUE ary, VALUE graph, VALUE source)
{
  long a = ARY_MAX_SIZE;
  long b = RARRAY_EMBED_LEN_MAX;
  long c = (int)sizeof(VALUE);
    fprintf(stderr, "*** rb_wgt_bfs_adj");
    VALUE trace = rb_ary_new2(10);
    queue_t queue = {0, 0, {0}};
    long member[100000] = {0};
    long node;
    enqueue(&queue, NUM2LONG(source));
    while (empty(&queue)) {
      node = dequeue(&queue);
      member[node] = 1;
      rb_ary_push(trace, LONG2NUM(node));
      VALUE adj = rb_ary_entry(graph, node);
      long n = RARRAY_LEN(adj);
      long i;
      for (i = 0; i < n; i++) {
        VALUE connected = rb_ary_entry(rb_ary_entry(graph, node),i);
        VALUE val = NUM2LONG(connected);
        if (val == 1) {
          if (member[i] == 0) {
            enqueue(&queue, i);
            member[i] = 1;
          };
        };
      };
    };
    return trace;
}

static
VALUE
rb_wgt_bfs(VALUE ary, VALUE source)
{
  clock_t begin, end;
  double time_spent;
  long size = RWIDGET(ary)->as.heap.len;
  long *graph = RWIDGET(ary)->as.heap.graph;
  long *path = ALLOC_N(long,size*size);

  /* grid_graph(size, graph); */

  begin = clock();

  /* long *graph = zeros(200); */
  /* assert(NULL != graph); */
  /* grid_graph(200, graph); */
  bfs99(size, graph, path, NUM2LONG(source));

  end = clock();
  time_spent = 1000*(double)(end - begin) / CLOCKS_PER_SEC;
  //printf("Time: %f", time_spent);

    VALUE trace = rb_ary_new();
  int i;
  for (i = 0; i < size*size; i++) {
    rb_ary_push(trace, LONG2NUM(path[i]));
  };

    return trace;
}

VALUE
rb_wgt_bfs_adj_better_membership(VALUE ary, VALUE graph, VALUE source)
{
    VALUE trace;
    VALUE queue;
    VALUE node;
    trace = rb_ary_new();
    queue = rb_ary_new();
    rb_ary_push(queue, source);
    long member[100000] = {0};
    while (RARRAY_LEN(queue) > 0) {
      node = rb_ary_shift(queue);
      long nd = NUM2LONG(node);
      member[nd] = 1;
      rb_ary_push(trace, node);
      VALUE adj;
      adj = rb_ary_entry(graph, NUM2LONG(node));
      long n = RARRAY_LEN(adj);
      long m = 1;
      long i;
      for (i = 0; i < n; i++) {
        VALUE connected = rb_ary_entry(rb_ary_entry(graph, NUM2LONG(node)),i);
        VALUE val = NUM2LONG(connected);
        if (val == 1) {
          VALUE idx = LONG2NUM(i);
          if (member[i] == 0) {
            rb_ary_push(queue, idx);
            member[i] = 1;
          };
        };
      };
    };
    return trace;
}

VALUE
rb_wgt_bfs_adj_slow(VALUE ary, VALUE graph, VALUE source)
{
    VALUE trace;
    VALUE queue;
    VALUE node;
    trace = rb_ary_new();
    queue = rb_ary_new();
    rb_ary_push(queue, source);
    while (RARRAY_LEN(queue) > 0) {
      node = rb_ary_shift(queue);
      long nd = NUM2LONG(node);
      rb_ary_push(trace, node);
      VALUE adj;
      adj = rb_ary_entry(graph, NUM2LONG(node));
      long n = RARRAY_LEN(adj);
      long m = 1;
      long i;
      for (i = 0; i < n; i++) {
        VALUE connected = rb_ary_entry(rb_ary_entry(graph, NUM2LONG(node)),i);
        VALUE val = NUM2LONG(connected);
        if (val == 1) {
          VALUE idx = LONG2NUM(i);
          if (!rb_ary_includes(trace, idx) && !rb_ary_includes(queue, idx)) {
            rb_ary_push(queue, idx);
          };
        };
      };
    };
    return trace;
}

static VALUE
rb_wgt_length(VALUE ary)
{
    long len = 0;
    long sum = 0;
    int i = 0;
    for (i = 0; i < 10000; i++) {
      sum += i;
    };
    return LONG2NUM(sum);
}

static VALUE
rb_wgt_test(VALUE ary)
{
  VALUE trace = rb_ary_new2(10);
  long n;
  for (n = 0; n < 19000; n++) {
    rb_ary_push(trace, LONG2NUM(n));
  }
  return trace;
}

static VALUE
rb_wgt_sum(VALUE ary, VALUE num)
{
    long sum, max;
    int i;

    sum = 0;
    max = NUM2LONG(num);
    for (i = 1; i <= max; i++) {
      sum += i;
    };
    return LONG2NUM(sum);
}

void
Init_Widget(void)
{
    rb_cWidget  = rb_define_class("Widget", rb_cObject);
    rb_define_alloc_func(rb_cWidget, wgt_alloc);

    rb_define_method(rb_cWidget, "initialize", rb_wgt_init, -1);
    rb_define_method(rb_cWidget, "inspect", rb_wgt_inspect, 0);
    rb_define_alias(rb_cWidget,  "to_s", "inspect");
    rb_define_method(rb_cWidget, "bfs", rb_wgt_bfs, 1);
    rb_define_method(rb_cWidget, "bfs_adj", rb_wgt_bfs_adj, 2);
    rb_define_method(rb_cWidget, "length", rb_wgt_length, 0);
    rb_define_method(rb_cWidget, "sum", rb_wgt_sum, 1);
}
