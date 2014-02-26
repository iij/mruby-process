module Process
  def self.waitpid2(pid, flags=0)
    i = waitpid(pid, flags)
    if i
      [i, $?.dup]
    else
      nil
    end
  end
end
