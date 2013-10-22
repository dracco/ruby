#include <ruby.h>

static VALUE cGraph;

typedef struct _node_t {
  long node;
  struct _node_t *next;
} node_t;


node_t **make_graph(long side)
{
  node_t **graph1 = malloc(side*side*sizeof(node_t*));

  long row, col;
  for (row = 0; row < side; row++)
  {
    for (col = 0; col < side; col++)
    {
      long node = row * side + col;
      if (row == 0)
      {
        if (col == 0)
        {
          graph1[node] = malloc(2 * sizeof(node_t));
          graph1[node]->node = node + 1;
          graph1[node]->next = &graph1[node][1];
          graph1[node][1].node = node + side;
          graph1[node][1].next = NULL;
        }
        else if (col == (side - 1))
        {
          graph1[node] = malloc(2 * sizeof(node_t));
          graph1[node]->node = node - 1;
          graph1[node]->next = &graph1[node][1];
          graph1[node][1].node = node + side;
          graph1[node][1].next = NULL;
        }
        else
        {
          graph1[node] = malloc(3 * sizeof(node_t));
          graph1[node]->node = node - 1;
          graph1[node]->next = &graph1[node][1];
          graph1[node][1].node = node + 1;
          graph1[node][1].next = &graph1[node][2];
          graph1[node][2].node = node + side;
          graph1[node][2].next = NULL;
        }
      }
      else if (row == (side - 1))
      {
        if (col == 0)
        {
          graph1[node] = malloc(2 * sizeof(node_t));
          graph1[node]->node = node - side;
          graph1[node]->next = &graph1[node][1];
          graph1[node][1].node = node + 1;
          graph1[node][1].next = NULL;
        }
        else if (col == (side - 1))
        {
          graph1[node] = malloc(2 * sizeof(node_t));
          graph1[node]->node = node - side;
          graph1[node]->next = &graph1[node][1];
          graph1[node][1].node = node - 1;
          graph1[node][1].next = NULL;
        }
        else
        {
          graph1[node] = malloc(3 * sizeof(node_t));
          graph1[node]->node = node - side;
          graph1[node]->next = &graph1[node][1];
          graph1[node][1].node = node - 1;
          graph1[node][1].next = &graph1[node][2];
          graph1[node][2].node = node + 1;
          graph1[node][2].next = NULL;
        }
      }
      else
      {
        if (col == 0)
        {
          graph1[node] = malloc(3 * sizeof(node_t));
          graph1[node]->node = node - side;
          graph1[node]->next = &graph1[node][1];
          graph1[node][1].node = node + 1;
          graph1[node][1].next = &graph1[node][2];
          graph1[node][2].node = node + side;
          graph1[node][2].next = NULL;
        }
        else if (col == (side -1 ))
        {
          graph1[node] = malloc(3 * sizeof(node_t));
          graph1[node]->node = node - side;
          graph1[node]->next = &graph1[node][1];
          graph1[node][1].node = node - 1;
          graph1[node][1].next = &graph1[node][2];
          graph1[node][2].node = node + side;
          graph1[node][2].next = NULL;
        }
        else
        {
          graph1[node] = malloc(4 * sizeof(node_t));
          graph1[node]->node = node - side;
          graph1[node]->next = &graph1[node][1];
          graph1[node][1].node = node - 1;
          graph1[node][1].next = &graph1[node][2];
          graph1[node][2].node = node + 1;
          graph1[node][2].next = &graph1[node][3];
          graph1[node][3].node = node + side;
          graph1[node][3].next = NULL;
        }
      }
    }
  }

  return graph1;
}

/* list_t */

typedef struct {
  long *list;
  long tail;
} list_t;

void list_append(list_t *list, long value)
{
  list->list[++list->tail] = value;
}

/* graph_t */

typedef struct _graph_t {
  long length;
  node_t **graph;
} graph_t;


/* queue_t */

typedef struct {
    long start;
    long end;
    long *queue;
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

long *bfs(long side, node_t **graph, long source)
{
  long *path = calloc(side*side, sizeof(long));
  long path_tail = 0;
  long *q = calloc(side*side, sizeof(long));
  queue_t queue = {0, 0, q};
  long *visited = calloc(side*side, sizeof(long));

  enqueue(&queue, source);
  visited[source] = 1;
  while(any(&queue))
  {
    long node = dequeue(&queue);
    path[path_tail++] = node;
    node_t *p = graph[node];
    while (p)
    {
      long i = p->node;
      if (visited[i] == 0)
      {
        enqueue(&queue, i);
        visited[i] = 1;
      }
      p = p->next;
    }
  }
  return path;
}

static
VALUE
graph_bfs(VALUE self, VALUE source)
{
  graph_t *graph;
  Data_Get_Struct(self, graph_t, graph);
  long side = graph->length;
  long *path;

  path = bfs(side, graph->graph, NUM2LONG(source));

  VALUE trace = rb_ary_new();

  rb_gc_register_address(trace); //# Mike D
  int i;
  for (i = 0; i < side*side; i++) {
    rb_ary_push(trace, LONG2NUM(path[i]));
  };
  rb_gc_unregister_address(trace); //# Mike D

  xfree(path);
  return trace;
}

static void
graph_mark(void *p) {
  //# mark any pure Ruby objects in use
}

static void
graph_free(void *p) {
  graph_t *graph = (graph_t*)p;
  xfree(graph->graph);
}

static VALUE
graph_alloc(VALUE klass) {
  VALUE self;
  graph_t *graph;
  self = Data_Make_Struct(klass, graph_t, graph_mark, graph_free, graph);

  return self;
}


static VALUE
graph_initialize(int argc, VALUE *argv, VALUE self) {
  graph_t *graph;
  Data_Get_Struct(self, graph_t, graph);
  if (argc == 1) {
    long side = NUM2LONG(argv[0]);
    graph->length = side;
    graph->graph = make_graph(side);
  };

  return self;
}

void
Init_adjacency_list_graph(void) {
  cGraph = rb_define_class("CAdjacencyListGraph", rb_cObject);

  rb_define_alloc_func(cGraph, graph_alloc);

  rb_define_method(cGraph, "initialize", graph_initialize, -1);
  rb_define_method(cGraph, "bfs", graph_bfs, 1);
}









