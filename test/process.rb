assert('Process') do
  Process.class == Module
end

assert('Process.kill') do
  # to be sure we won't kill an innocent process!
  Process.pid > 0 and Process.kill(0, Process.pid)
end

assert('Process.pid') do
  Process.pid.is_a?(Integer) and Process.pid > 0
end

assert('Process.ppid') do
  Process.ppid.is_a?(Integer) and Process.ppid > 0
end

assert('$$') do
  $$ == Process.pid
end

assert("Process.fork with WNOHANG") do
  pid = fork {
    loop {}
  }
  p, s = Process.waitpid2(pid, Process::WNOHANG)
  assert_nil(p)
  assert_nil(s)

  Process.kill :TERM, pid
  loop {
    # wait until the process completely killed with non-block mode
    p, s = Process.waitpid2(pid, Process::WNOHANG)
    break if p
  }
  assert_equal(pid, p)
  assert_true(s.signaled?)
end
