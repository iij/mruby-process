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

class String
  def strip
    a = 0
    z = size - 1
    a += 1 while a <= z and " \f\n\r\t\v".include?(self[a])
    z -= 1 while a <= z and " \f\n\r\t\v\0".include?(self[z])
    z >= 0 ? self[a..z] : ''
  end
end unless ''.respond_to? :strip

def IO.sysopen(path, mod)
  ProcessTest.sysopen(path, mod)
end if OS.windows?

def read(path)
  f = File.open(path)
  f.read.to_s.strip
ensure
  f.close if f
end

def wait_for_pid(pid)
  loop do
    p = Process.waitpid(pid, Process::WNOHANG)
    break if p
  end
end

def assert_not_windows(*args, &block)
  assert(*args, &block) if OS.posix?
end

def assert_windows(*args, &block)
  assert(*args, &block) if OS.windows?
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

assert('Process.spawn') do
  assert_raise(ArgumentError) { spawn }
  assert_raise(TypeError) { spawn 123 }

  # This test fails on travis (returns status 0 even cmd does not exist)
  assert_raise(RuntimeError) { wait_for_pid spawn('.exe') } unless ENV['TRAVIS']

  pid = spawn 'exit 0'
  wait_for_pid(pid)

  assert_kind_of Integer, pid
  assert_true pid > 0
  assert_not_equal $PID, pid
  assert_kind_of Process::Status, $?
  assert_equal $?.pid, pid

  var = "#{ENV['RAND']}x"
  pid = spawn("echo #{var} > tmp/spawn.txt")

  wait_for_pid(pid)
  assert_equal var, read('tmp/spawn.txt')
end

assert('Process.spawn', 'env') do
  var = "x#{ENV['RAND']}"
  env = OS.posix? ? '$MYVAR' : '%MYVAR%'
  pid = spawn({ MYVAR: var }, "echo #{env} > tmp/spawn.txt")

  wait_for_pid(pid)
  assert_equal var, read('tmp/spawn.txt')
end

# TODO: More tests for edge cases! whatif no valid pipe,
assert('Process.spawn', 'pipe') do
  begin
    var = ENV['RAND']
    pip = IO.sysopen('tmp/pipe.txt', 'w')
    pid = spawn("echo #{var}", out: pip)

    wait_for_pid(pid)
    assert_equal var, read('tmp/pipe.txt')

    env = OS.posix? ? '$MYVAR' : '%MYVAR%'
    pid = spawn({ MYVAR: var }, "echo #{env}", out: pip)

    wait_for_pid(pid)

    assert_equal var * 2, read('tmp/pipe.txt').sub("\r", '').sub("\n", '')
  ensure
    IO._sysclose(pip) if OS.posix?
  end
end

assert('Process.spawn', 'pipe error') do
  begin
    pip = IO.sysopen('tmp/pipe_error.txt', 'w')
    pid = spawn("ls -asdw", err: pip)

    wait_for_pid(pid)
    assert_include read('tmp/pipe_error.txt'), "ls: option requires an argument"
  ensure
    IO._sysclose(pip) if OS.posix?
  end
end

assert('Process.exec', 'invalid signatures') do
  assert_raise(ArgumentError) { exec }
  assert_raise(TypeError)     { exec 123 }
end

assert_not_windows('Process.exec') do
  var = ENV['RAND']
  pid = fork { exec({ MYVAR: var }, 'echo $MYVAR > tmp/exec.txt') }

  wait_for_pid(pid)
  assert_equal var, read('tmp/exec.txt')

  var = "x#{var}"
  pid = fork { exec '/bin/sh', '-c', "echo #{var} > tmp/exec.txt" }

  wait_for_pid(pid)
  assert_equal var, read('tmp/exec.txt')
end

assert_not_windows('Process.exec', '$SHELL') do
  ['/bin/bash', '/bin/sh'].each do |shell|
    ENV['SHELL'] = shell

    pid = fork { exec 'echo $SHELL > tmp/exec.txt' }
    wait_for_pid(pid)

    assert_equal shell, read('tmp/exec.txt')
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

assert('Process.wait2') do
  pid     = spawn('sleep 2')
  p, st   = Process.waitpid2(pid, Process::WNOHANG)

  assert_nil p
  assert_nil st

  Process.kill :KILL, pid

  loop do
    p, st = Process.waitpid2(pid, Process::WNOHANG)
    break if p
  end

  assert_equal pid, p
  assert_kind_of Process::Status, st
  assert_include [9, nil], st.termsig
end

assert('Process.waitall') do
  assert_true Process.waitall.empty?

  pids = []
  pids << spawn('exit 2')
  pids << spawn('exit 1')
  pids << spawn('exit 0')

  a = Process.waitall

  pids.each do |pid|
    assert_raise(RuntimeError) { Process.kill(0, pid) }
  end

  assert_kind_of Array, a
  assert_equal 3, a.size

  pids.each do |pid|
    pid_status = a.find { |i| i[0] == pid }

    assert_kind_of Array, pid_status
    assert_equal 2, pid_status.size
    assert_equal pid, pid_status.first
    assert_kind_of Process::Status, pid_status.last
  end
end

assert('Process.system') do
  assert_raise(ArgumentError) { system }
  assert_raise(TypeError)     { system 123 }

  assert_true  system 'exit 0'
  assert_equal 0, $?.exitstatus
  assert_false system 'exit 1'
  assert_equal 1, $?.exitstatus

  assert_nothing_raised { system 'exit' }

  var = ENV['RAND']
  env = OS.posix? ? '$MYVAR' : '%MYVAR%'

  system({ MYVAR: var }, "echo #{env} > tmp/system.txt")

  assert_equal var, read('tmp/system.txt')
end

assert_windows('Process.fork') do
  assert_false Process.respond_to? :fork
  assert_false Kernel.respond_to? :fork
end
