#include <ruby.h>

static VALUE
rb_task_for_pid(VALUE klass, VALUE pid)
{
    task_t port;
    task_for_pid(mach_task_self(), NUM2INT(pid), &port);

    return Qnil;
}

void Init_heapdump()
{
    VALUE mHeap = rb_define_class("Heap", rb_cObject);
    rb_define_method(mHeap, "task_for_pid", rb_task_for_pid, 2);
}

/* vim: set noet sws=4 sw=4: */
