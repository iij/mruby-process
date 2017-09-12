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

module Process
  # Process::Status encapsulates the information on the status of a running or
  # terminated system process. The built-in variable $? is either nil or a
  # Process::Status object.
  class Status
    # Encapsulates the information on the status of a system process.
    def initialize(pid, status)
      @pid    = pid
      @status = status
    end

    # Returns true if the integer value of stat equals other.
    #
    # @return [ String ]
    def ==(other)
      to_i == other.to_i
    end

    # Shift the bits in stat right num places.
    #
    # @return [ Integer ]
    def >>(other)
      to_i >> other
    end

    # Override the inspection method.
    def inspect
      "#<Process::Status: pid #{@pid} exit #{@status}>"
    end

    # Returns the process ID that this status object represents.
    attr_reader :pid

    # Returns true if status is successful, false if not.
    # Returns nil if exited? is not true.
    #
    # @return [ Boolean ]
    def success?
      return nil unless exited?
      exitstatus == 0
    end

    # The bits in status as a Integer.
    #
    # @return [ Integer ]
    def to_i
      @status
    end

    alias to_int to_i

    # Show pid and exit status.
    #
    # @return [ String ]
    def to_s
      "pid #{@pid} exit #{@status}"
    end
  end
end
