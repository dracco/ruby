#include <ruby.h>

static VALUE cGraph;

typedef struct _graph {
  long lenght;
  long *graph;
} Graph;

static void graph_mark(void *p) {
  printf("*** graph_mark\n");
}

static void graph_free(void *p) {
  printf("*** graph_free\n");
  Graph *graph = p;
  printf("Lenght: %li\n", graph->lenght);
  xfree(graph->graph);
}


static VALUE graph_alloc(VALUE klass) {
  VALUE obj;
  Graph *graph;
  obj = Data_Make_Struct(klass, Graph, graph_mark, graph_free, graph);
  graph->lenght = 12;
  printf("Lenght: %li\n", graph->lenght);
  return obj;
}

void Init_alloc(void) {
  printf("alloc\n");
  cGraph = rb_define_class("Alloc", rb_cObject);
  rb_define_alloc_func(cGraph, graph_alloc);
}
