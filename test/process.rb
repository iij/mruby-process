assert('Process') do
  Process.class == Module
end

assert('Process.pid') do
  Process.pid is_a? Integer and Process.pid > 0
end

assert('Process.ppid') do
  Process.ppid is_a? Integer and Process.ppid > 0
end

assert('Process.kill') do
  # to be sure we won't kill an innocent process!
  Process.pid > 0 and Process.kill(0, Process.pid)
end

assert('$$') do
  $$ == Process.pid
end
