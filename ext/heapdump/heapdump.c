#include <ruby.h>
#include <mach/mach.h>
#include <sys/types.h>
#include <sys/ptrace.h>

VALUE cHeapThread;
VALUE cHeapThreadState64;

#define ASSERT_SUCCESS(_stmt) \
do { \
    int ret = _stmt; \
    if (KERN_SUCCESS != ret) \
      rb_raise(rb_eRuntimeError, "%s", mach_error_string(ret));\
} while(0)

static void dealloc(void *ptr)
{
    xfree(ptr);
}

static const rb_data_type_t HeapTask_type = {
    "Heap::Task",
    { 0, dealloc, 0, },
    0, 0,
#ifdef RUBY_TYPED_FREE_IMMEDIATELY
    RUBY_TYPED_FREE_IMMEDIATELY,
#endif
};

static const rb_data_type_t ThreadState64_type = {
    "Heap::Thread::State64",
    { 0, xfree, 0, },
    0, 0,
#ifdef RUBY_TYPED_FREE_IMMEDIATELY
    RUBY_TYPED_FREE_IMMEDIATELY,
#endif
};

static VALUE
rb_thread_count_for_task(VALUE self)
{
    mach_port_t * task;
    thread_act_port_array_t thread_list;
    mach_msg_type_number_t thread_count;

    TypedData_Get_Struct(self, mach_port_t, &HeapTask_type, task);

    ASSERT_SUCCESS(task_threads(*task, &thread_list, &thread_count));
    return INT2NUM(thread_count);
}

static VALUE
rb_thread_state_at(VALUE self, VALUE idx)
{
    mach_port_t * task;
    thread_act_port_array_t thread_list;
    mach_msg_type_number_t thread_count;
    x86_thread_state_t * state;
    mach_msg_type_number_t state_count = x86_THREAD_STATE_COUNT;
    VALUE state_struct;

    TypedData_Get_Struct(self, mach_port_t, &HeapTask_type, task);

    state = xcalloc(1, sizeof(x86_thread_state_t));

    ASSERT_SUCCESS(task_threads(*task, &thread_list, &thread_count));

    if(NUM2INT(idx) >= thread_count || NUM2INT(idx) < 0) {
	rb_raise(rb_eRuntimeError, "index out of bounds");
    }

    ASSERT_SUCCESS(thread_get_state(thread_list[NUM2INT(idx)], x86_THREAD_STATE, state, &state_count));

    return TypedData_Wrap_Struct(cHeapThreadState64, &ThreadState64_type, state);
}

static VALUE
rb_task_for_pid(VALUE klass, VALUE pid)
{
    mach_port_t * task;

    task = xmalloc(sizeof(mach_port_t));

    ASSERT_SUCCESS(task_for_pid(mach_task_self(), NUM2INT(pid), task));

    return TypedData_Wrap_Struct(klass, &HeapTask_type, task);

}

#define STATE_64(_member) \
static VALUE rb_thread_state_##_member(VALUE self) \
{ \
    x86_thread_state_t * state;\
    TypedData_Get_Struct(self, x86_thread_state_t, &ThreadState64_type, state); \
    return LONG2NUM(state->uts.ts64.__##_member); \
}

STATE_64(rax);
STATE_64(rbx);
STATE_64(rcx);
STATE_64(rdx);
STATE_64(rdi);
STATE_64(rsi);
STATE_64(rbp);
STATE_64(rsp);
STATE_64(r8);
STATE_64(r9);
STATE_64(r10);
STATE_64(r11);
STATE_64(r12);
STATE_64(r13);
STATE_64(r14);
STATE_64(r15);
STATE_64(rip);
STATE_64(rflags);
STATE_64(cs);
STATE_64(fs);
STATE_64(gs);

#undef STATE_64

void Init_heapdump()
{
    VALUE mHeap = rb_define_module("Heap");
    VALUE cTask = rb_define_class_under(mHeap, "Task", rb_cObject);
    cHeapThread = rb_define_class_under(mHeap, "Thread", rb_cObject);
    cHeapThreadState64 = rb_define_class_under(cHeapThread, "State64", rb_cObject);

    rb_define_singleton_method(cTask, "for_pid", rb_task_for_pid, 1);
    rb_define_method(cTask, "thread_count", rb_thread_count_for_task, 0);
    rb_define_method(cTask, "thread_state_at", rb_thread_state_at, 1);

#define STATE_64(_member) \
    rb_define_method(cHeapThreadState64, #_member, rb_thread_state_##_member, 0);

    STATE_64(rax);
    STATE_64(rbx);
    STATE_64(rcx);
    STATE_64(rdx);
    STATE_64(rdi);
    STATE_64(rsi);
    STATE_64(rbp);
    STATE_64(rsp);
    STATE_64(r8);
    STATE_64(r9);
    STATE_64(r10);
    STATE_64(r11);
    STATE_64(r12);
    STATE_64(r13);
    STATE_64(r14);
    STATE_64(r15);
    STATE_64(rip);
    STATE_64(rflags);
    STATE_64(cs);
    STATE_64(fs);
    STATE_64(gs);
}

/* vim: set noet sws=4 sw=4: */
