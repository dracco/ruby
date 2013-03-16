#include <ruby.h>

static VALUE cGraph;

typedef struct _graph {
  long lenght;
  long *graph;
} Graph;


/* queue_t */

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

static
VALUE
graph_bfs(VALUE self, VALUE source)
{
  Graph *graph;
  Data_Get_Struct(self, Graph, graph);
  clock_t begin, end;
  double time_spent;
  long size = graph->lenght;
  /* long *graph = graph->graph; */
  long *path = ALLOC_N(long,size*size);

  /* grid_graph(size, graph); */

  begin = clock();

  /* long *graph = zeros(200); */
  /* assert(NULL != graph); */
  /* grid_graph(200, graph); */
  bfs99(size, graph->graph, path, NUM2LONG(source));

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

static void graph_mark(void *p) {
  printf("*** graph_mark\n");
}

static void graph_free(void *p) {
  printf("*** graph_free\n");
  Graph *graph = p;
  printf("free Lenght: %li\n", graph->lenght);
  /* xfree(graph->graph); */
}

static VALUE graph_alloc(VALUE klass) {
  VALUE obj;
  Graph *graph;
  obj = Data_Make_Struct(klass, Graph, graph_mark, graph_free, graph);
  graph->lenght = 12;
  printf("alloc Lenght: %li\n", graph->lenght);
  return obj;
}

static VALUE graph_initialize(int argc, VALUE *argv, VALUE self) {
  Graph *graph;
  Data_Get_Struct(self, Graph, graph);
  printf("initialie Lenght: %li\n", graph->lenght);
    if (argc == 1) {
      long size = NUM2LONG(argv[0]);
      graph->lenght = size;
      graph->graph = ALLOC_N(long, size*size*size*size);
      /* long *graph = zeros(size); */

      grid_graph(size, graph->graph);
      /* RWIDGET(ary)->as.heap.graph = graph; */
    };
    /* return ary; */
  return self;
}


void Init_alloc(void) {
  printf("alloc\n");
  cGraph = rb_define_class("Alloc", rb_cObject);
  rb_define_alloc_func(cGraph, graph_alloc);
  rb_define_method(cGraph, "initialize", graph_initialize, -1);
  rb_define_method(cGraph, "bfs", graph_bfs, 1);
}
