#include <ruby.h>

static VALUE
rb_hello_select(VALUE self)
{
    VALUE result = rb_yield_values(2, LONG2NUM(3), LONG2NUM(2));
    return result;
}

void Init_hello(void) {
  printf("Hi");
  VALUE klass = rb_define_class("Hello", rb_cObject);

  rb_define_method(klass, "select", rb_hello_select, 0);
}
