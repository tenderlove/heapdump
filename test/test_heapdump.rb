require 'minitest/autorun'
require 'heapdump.so'

class TestHeapdump < Minitest::Test
  def test_works
    task = Heap::Task.for_pid $$
    task.thread_count.times do |i|
      p :THREAD => i
      state = task.thread_state_at(i)

      members = %i{ rax rbx rcx rdx rdi rsi rbp rsp r8 r9 r10 r11 r12 r13 r14 r15 rip rflags cs fs gs }

      puts members.map { |v| [v, "%x" % state.send(v)].join(": ") }.join(' ')
    end
  end
end
