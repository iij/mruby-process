mruby-process
=========
[![Build Status](https://travis-ci.org/iij/mruby-process.svg?branch=master)](https://travis-ci.org/iij/mruby-process)


## install by mrbgems
 - add conf.gem line to `build_config.rb`
```ruby
MRuby::Build.new do |conf|

    # ... (snip) ...

    conf.gem :git => 'https://github.com/iij/mruby-process.git'
end
```

## Features

 - Process - fork kill pid ppid waitpid waitpid2
 - Process::Status - all methods but `&`, ``>>``
   - You can use ``Process::Status.new(pid, status)`` to set ``$?`` in
     your script or other mrbgems.
 - Kernel - $$ exit exit! fork sleep system


## Caveats

 - $? may not work correctly on the platform where ``pid_t`` is not ``int`` or
   ``MRB_INT_MAX`` is less than ``PID_MAX`` (or /proc/sys/kernel/pid_max).


## License

Copyright (c) 2012 Internet Initiative Japan Inc.

Permission is hereby granted, free of charge, to any person obtaining a 
copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation 
the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the 
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in 
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
DEALINGS IN THE SOFTWARE.

