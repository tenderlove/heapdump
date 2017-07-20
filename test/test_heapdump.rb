require 'minitest/autorun'
require 'heapdump.so'

class TestHeapdump < Minitest::Test
  def test_sanity
    Heap.new.task_for_pid(ENV["XPID"].to_i)
  end
end
