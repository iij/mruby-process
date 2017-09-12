# MIT License
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

def assert_not_windows(*args, &block)
  assert(*args, &block) if OS.posix?
end

def spawn_and_wait(cmd = 'exit 0')
  Process.waitpid2(spawn(cmd))
end

assert('$?') do
  _, st = spawn_and_wait

  assert_kind_of Process::Status, $?
  assert_equal st, $?
end

assert('$CHILD_STATUS') do
  spawn_and_wait

  assert_kind_of Process::Status, $CHILD_STATUS
  assert_equal $?, $CHILD_STATUS
end

assert('Process::Status#pid') do
  pid, st = spawn_and_wait

  assert_equal pid, st.pid
end

assert('Process::Status#==') do
  _, st1 = spawn_and_wait
  _, st2 = spawn_and_wait

  assert_equal st1, st2
end

assert('Process::Status#>>') do
  spawn_and_wait 'exit 99'

  assert_equal 99, $? >> 8
end

# assert('Process::Status#coredump?')

assert('Process::Status#exited?') do
  spawn_and_wait

  assert_true $?.exited?
end

assert('Process::Status#exitstatus') do
  spawn_and_wait 'exit 42'

  assert_equal 42, $?.exitstatus
end

assert_not_windows('Process::Status#signaled? and #termsig') do
  pid = fork

  sleep 10 unless pid
  Process.kill 15, pid

  _, st = Process.waitpid2(pid)

  assert_true st.signaled?
  assert_equal 15, st.termsig
end

# assert('Process::Status#stopped and #stopsig')

assert('Process::Status#success?') do
  spawn_and_wait 'exit 42'

  assert_true $?.exited?
  assert_equal 42, $?.exitstatus
end

assert('Process::Status#to_i') do
  assert_kind_of Integer, $?.to_i
  assert_kind_of Integer, $?.to_int
  assert_equal $?.to_i, $?.to_int
end

assert('Process::Status#to_s') do
  pid, = spawn_and_wait

  assert_equal "pid #{pid} exit 0", $?.to_s
end

assert('Process::Status#inspect') do
  pid, = spawn_and_wait

  assert_equal "#<Process::Status: pid #{pid} exit 0>", $?.inspect
end
