def process_status_fork(exitcode=0)
  pid = fork
  unless pid
    exit! exitcode
  end
  pid
end

assert('Process::Status#==') do
  pid1 = process_status_fork 123
  pid2 = process_status_fork 123
  _, st1 = Process.waitpid2(pid1)
  _, st2 = Process.waitpid2(pid2)
  assert_true st1 == st2
end

# assert('Process::Status#coredump?') do

assert('Process::Status#exited? and #exitstatus') do
  pid = process_status_fork 42
  _, st = Process.waitpid2(pid)
  assert_true st.exited?
  assert_equal 42, st.exitstatus
end

# assert('Process::Status#inspect') do

assert('Process::Status#pid') do
  pid = process_status_fork
  _, st = Process.waitpid2(pid)
  assert_equal pid, st.pid
end

assert('Process::Status#signaled? and #termsig') do
  pid = fork
  unless pid
    sleep 10
  end
  Process.kill 15, pid
  _, st = Process.waitpid2(pid)
  assert_true st.signaled?
  assert_equal 15, st.termsig
end

# assert('Process::Status#stopped and #stopsig') do

assert('Process::Status#success?') do
  pid = process_status_fork 42
  _, st = Process.waitpid2(pid)
  assert_true st.exited?
  assert_equal 42, st.exitstatus
end

assert('Process::Status#to_i, to_int, to_s') do
  pid = process_status_fork 123
  _, st = Process.waitpid2(pid)
  assert_true st.to_i.is_a? Fixnum
  assert_true st.to_int.is_a? Fixnum
  assert_equal st.to_i.to_s, st.to_s
end
