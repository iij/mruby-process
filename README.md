mruby-process [![codebeat badge](https://codebeat.co/badges/02e9d761-e8b6-4939-9ef2-b88fd0c93a84)](https://codebeat.co/projects/github-com-appplant-mruby-process-windows) [![Build Status](https://travis-ci.org/appPlant/mruby-process.svg?branch=windows)](https://travis-ci.org/appPlant/mruby-process) [![Build status](https://ci.appveyor.com/api/projects/status/1uu04u7wtbup1oqk/branch/windows?svg=true)](https://ci.appveyor.com/project/katzer/mruby-process/branch/windows) 
=============


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

 ## Implemented methods

### Process
 - https://ruby-doc.org/core-2.2.0/Process.html

| method                     | mruby-process |
| -------------------------  | :-----------: |
| Process.abort              |       o       |
| Process.argv0              |       o       |
| Process.clock_getres       |               |
| Process.clock_gettime      |               |
| Process.daemon             |               |
| Process.detach             |               |
| Process.egid               |               |
| Process.egid=              |               |
| Process.euid               |               |
| Process.euid=              |               |
| Process.exec               |       o       |
| Process.exit               |       o       |
| Process.exit!              |       o       |
| Process.fork               |       o       |
| Process.getpgid            |               |
| Process.getpgrp            |               |
| Process.getpriority        |               |
| Process.getrlimit          |               |
| Process.getsid             |               |
| Process.gid                |               |
| Process.gid=               |               |
| Process.groups             |               |
| Process.groups=            |               |
| Process.initgroups         |               |
| Process.kill               |       o       |
| Process.maxgroups          |               |
| Process.maxgroups=         |               |
| Process.pid                |       o       |
| Process.ppid               |       o       |
| Process.setpgid            |               |
| Process.setpgrp            |               |
| Process.setpriority        |               |
| Process.setproctitle       |               |
| Process.setrlimit          |               |
| Process.setsid             |               |
| Process.spawn              |       o       |
| Process.times              |               |
| Process.uid                |               |
| Process.uid=               |               |
| Process.wait               |       o       |
| Process.wait2              |       o       |
| Process.waitall            |       o       |
| Process.waitpid            |       o       |
| Process.waitpid2           |       o       |

     
### Kernel

  - https://ruby-doc.org/core-2.4.1/Kernel.html

| method                     | mruby-process |
| -------------------------  | :-----------: |
|\#`                         |               |
|\#abort                     |       o       |
|\#at_exit                   |               |
|\#autoload                  |               |
|\#autoload?                 |               |
|\#binding                   |               |
|\#block_given?              |               |
|\#callcc                    |               |
|\#caller                    |               |
|\#caller_locations          |               |
|\#catch                     |               |
|\#chomp                     |               |
|\#chop                      |               |
|\#eval                      |               |
|\#exec                      |       o       |
|\#exit                      |       o       |
|\#exit!                     |       o       |
|\#fail                      |               |
|\#fork                      |               |
|\#format                    |               |
|\#gets                      |               |
|\#global_variables          |               |
|\#gsub                      |               |
|\#iterator?                 |               |
|\#lambda                    |               |
|\#load                      |               |
|\#local_variables           |               |
|\#loop                      |               |
|\#open                      |               |
|\#p                         |               |
|\#print                     |               |
|\#printf                    |               |
|\#proc                      |               |
|\#putc                      |               |
|\#puts                      |               |
|\#raise                     |               |
|\#rand                      |               |
|\#readline                  |               |
|\#readlines                 |               |
|\#require                   |               |
|\#require_relative          |               |
|\#select                    |               |
|\#set_trace_func            |               |
|\#sleep                     |               |
|\#spawn                     |       o       |
|\#sprintf                   |               |
|\#srand                     |               |
|\#sub                       |               |
|\#syscall                   |               |
|\#system                    |               |
|\#test                      |               |
|\#throw                     |               |
|\#trace_var                 |               |
|\#trap                      |               |
|\#untrace_var               |               |
|\#warn                      |               |



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

