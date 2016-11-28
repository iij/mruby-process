assert("Process.fork with WNOHUNG") do
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
