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

def wait_for_pid(pid)
  loop do
    p = Process.waitpid(pid, Process::WNOHANG)
    break if p
  end
end

def assert_not_windows(str, &block)
  assert(str, &block) if OS.posix?
end

def assert_windows(str, &block)
  assert(str, &block) if OS.windows?
end

assert('Process') do
  assert_kind_of Module, Process
end

assert('Process::WNOHANG') do
  assert_kind_of Integer, Process::WNOHANG
end

assert('Process::WUNTRACED') do
  assert_kind_of Integer, Process::WUNTRACED
end

assert_not_windows('Process.argv0') do
  assert_equal ENV['_'], Process.argv0
end

assert_windows('Process.argv0') do
  assert_include Process.argv0, 'mrbtest.exe'
end

assert('$0') do
  assert_raise(RuntimeError, 'Should be frozen') { $0.upcase! }
  assert_not_include ['/', '\\'], $0
end

assert('$PROGRAM_NAME') do
  assert_raise(RuntimeError, 'Should be frozen') { $PROGRAM_NAME.upcase! }
  assert_equal $0, $PROGRAM_NAME
end

assert('Process.pid') do
  assert_kind_of Integer, Process.pid
  assert_true Process.pid > 0
end

assert('$$') do
  assert_equal Process.pid, $$
end

assert('$PID') do
  assert_equal Process.pid, $PID
end

assert('$PROCESS_ID') do
  assert_equal Process.pid, $PROCESS_ID
end

assert('Process.ppid') do
  assert_kind_of Integer, Process.pid
  assert_true Process.pid > 0
end

assert_not_windows('Process.exec') do
  assert_raise(ArgumentError) { exec }
  assert_raise(TypeError) { exec 123 }

  assert_raise(RuntimeError) { exec 'echo *', '123' }
  assert_raise(RuntimeError) { exec '' }

  var = Time.now.to_i.to_s
  pid = fork { exec({ MYVAR: var }, 'echo $MYVAR > ../tmp/exec.txt') }

  wait_for_pid(pid)

  File.open('../tmp/exec.txt') do |f|
    assert_equal var, f.read.chomp
  end

  var = "x#{var}"
  pid = fork { exec '/bin/sh', '-c', "echo #{var} > ../tmp/exec.txt" }

  wait_for_pid(pid)

  File.open('../tmp/exec.txt') do |f|
    assert_equal var, f.read.chomp
  end
end

assert_not_windows('Process.exec /shell') do
  ['/bin/bash', '/bin/sh'].each do |shell|
    ENV['SHELL'] = shell

    pid = fork { exec 'echo $SHELL > ../tmp/exec.txt' }

    wait_for_pid(pid)

    File.open('../tmp/exec.txt') do |f|
      assert_equal shell, f.read.chomp
    end
  end
end

assert('Process.kill') do
  assert_nothing_raised { Process.kill(:EXIT, Process.pid) }
  assert_nothing_raised { Process.kill('EXIT', Process.pid) }
  assert_nothing_raised { Process.kill(0, Process.pid) }
  assert_equal 1, Process.kill(0, Process.pid), 'killed an innocent process'
  assert_equal 2, Process.kill(0, Process.pid, Process.pid)
  assert_raise(TypeError) { Process.kill(0.0, Process.pid) }
  assert_raise(TypeError) { Process.kill(0, 'Process.pid') }
  assert_raise(ArgumentError) { Process.kill(:UNKNOWN, Process.pid) }
end

assert_not_windows('Process.wait2') do
  pid  = fork { loop {} }
  p, s = Process.waitpid2(pid, Process::WNOHANG)

  assert_nil(p)
  assert_nil(s)

  Process.kill :TERM, pid

  loop do
    p, s = Process.waitpid2(pid, Process::WNOHANG)
    break if p
  end

  assert_equal(pid, p)
  assert_kind_of(Process::Status, s)
  assert_true(s.signaled?)
end

assert_not_windows('Process.waitall') do
  assert_true Process.waitall.empty?

  pids = []
  pids << fork { exit! 2 }
  pids << fork { exit! 1 }
  pids << fork { exit! 0 }

  a = Process.waitall

  pids.each do |pid|
    assert_raise(RuntimeError) { Process.kill(0, pid) }
  end

  assert_kind_of Array, a
  assert_equal 3, a.size

  pids.each do |pid|
    pid_status = a.assoc(pid)

    assert_kind_of Array, pid_status
    assert_equal 2, pid_status.size
    assert_equal pid, pid_status.first
    assert_kind_of Process::Status, pid_status.last
  end
end

assert_windows('Process.fork') do
  assert_false Process.respond_to? :fork
  assert_false Kernel.respond_to? :fork
end
