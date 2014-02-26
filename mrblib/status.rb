module Process
  class Status
    def initialize(pid, status)
      @pid = pid
      @status = status
    end

    def ==(other)
      self.to_i == other.to_i
    end

    def inspect
      if exited?
        s = "exited(#{exitstatus})"
      elsif stopped?
        s = "stopped(#{stopsig})"
      elsif signaled?
        if coredump?
          s = "coredumped"
        else
          s = "signaled(#{termsig})"
        end
      else
        s = "status=#{@status}"
      end
      "#<Process::Status: pid=#{@pid},#{s}>"
    end

    attr_reader :pid

    def success?
      self.exitstatus == 0
    end

    def to_i
      @status
    end
    alias to_int to_i

    def to_s
      self.to_i.to_s
    end
  end
end
