# mruby-process <br> [![Build Status](https://travis-ci.org/appPlant/mruby-process.svg?branch=windows)](https://travis-ci.org/appPlant/mruby-process) [![Build status](https://ci.appveyor.com/api/projects/status/1uu04u7wtbup1oqk/branch/windows?svg=true)](https://ci.appveyor.com/project/katzer/mruby-process/branch/windows) [![codebeat badge](https://codebeat.co/badges/02e9d761-e8b6-4939-9ef2-b88fd0c93a84)](https://codebeat.co/projects/github-com-appplant-mruby-process-windows)

Implementation of the Ruby 2.4.1 Core Library _Process_ for [mruby][mruby].

All listed methods have been tested with Ubuntu, MacOS and Windows. Cross compilation works as well.

```ruby
pid = spawn('ls', '-la')
```

Include [process.h][process_h] to use the native methods within your own project:

```c
// To use kill, waitpid, fork, spawn, ... on Unix and Win32

#ifdef HAVE_MRB_PROCESS_H
# include 'process.h'
#endif

static pid_t
spawn_process(const char *path, char *const argv[], char *const envp[])
{
    return spawnve(path, argv, envp);
}
```

## Installation

Add the line below to your `build_config.rb`:

```ruby
MRuby::Build.new do |conf|
  # ... (snip) ...
  conf.gem 'mruby-process'
end
```

Or add this line to your aplication's `mrbgem.rake`:

```ruby
MRuby::Gem::Specification.new('your-mrbgem') do |spec|
  # ... (snip) ...
  spec.add_dependency 'mruby-process'
end
```


## Implemented methods

### Process

- https://ruby-doc.org/core-2.4.1/Process.html

| method                     | mruby-process | Comment |
| -------------------------  | :-----------: | :------ |
| $0                         |       o       |
| $PROGRAM_NAME              |       o       |
| $$                         |       o       |
| $PID                       |       o       |
| $PROCESS_ID                |       o       |
| ::WNOHANG                  |       o       |
| ::WUNTRACED                |       o       |
| #abort                     |       o       |
| #argv0                     |       o       |
| #clock_getres              |               |
| #clock_gettime             |               |
| #daemon                    |               |
| #detach                    |               |
| #egid                      |               | Implemented in [mruby-process-ext][mruby-process-ext] |
| #egid=                     |               | Implemented in [mruby-process-sys][mruby-process-sys] |
| #euid                      |               | Implemented in [mruby-process-ext][mruby-process-ext] |
| #euid=                     |               | Implemented in [mruby-process-sys][mruby-process-sys] |
| #exec                      |       o       |
| #exit                      |       o       |
| #exit!                     |       o       |
| #fork                      |       o       | If fork is not usable, Process.respond_to?(:fork) returns false. |
| #getpgid                   |               |
| #getpgrp                   |               |
| #getpriority               |               |
| #getrlimit                 |               |
| #getsid                    |               |
| #gid                       |               | Implemented in [mruby-process-ext][mruby-process-ext] |
| #gid=                      |               | Implemented in [mruby-process-sys][mruby-process-sys] |
| #groups                    |               | Implemented in [mruby-process-sys][mruby-process-sys] |
| #groups=                   |               | Implemented in [mruby-process-sys][mruby-process-sys] |
| #initgroups                |               |
| #kill                      |       o       |
| #maxgroups                 |               |
| #maxgroups=                |               |
| #pid                       |       o       |
| #ppid                      |       o       |
| #setpgid                   |               |
| #setpgrp                   |               |
| #setpriority               |               |
| #setproctitle              |               |
| #setrlimit                 |               |
| #setsid                    |               |
| #spawn                     |       o       |
| #times                     |               |
| #uid                       |               | Implemented in [mruby-process-ext][mruby-process-ext] |
| #uid=                      |               | Implemented in [mruby-process-ext][mruby-process-ext] |
| #wait                      |       o       |
| #wait2                     |       o       |
| #waitall                   |       o       |
| #waitpid                   |       o       |
| #waitpid2                  |       o       |


### Process::Status

- https://ruby-doc.org/core-2.4.1/Process/Status.html

| method                     | mruby-process |
| -------------------------  | :-----------: |
| $?                         |       o       |
| $CHILD_STATUS              |       o       |
| #&                         |               |
| #==                        |       o       |
| #>>                        |       o       |
| #coredump?                 |       o       |
| #exited?                   |       o       |
| #exitstatus                |       o       |
| #inspect                   |       o       |
| #pid                       |       o       |
| #signaled?                 |       o       |
| #stopped?                  |       o       |
| #stopsig                   |       o       |
| #success?                  |       o       |
| #termsig                   |       o       |
| #to_i                      |       o       |
| #to_s                      |       o       |

     
### Kernel

- https://ruby-doc.org/core-2.4.1/Kernel.html

| method                     | mruby-process | Comment |
| -------------------------  | :-----------: | :-----  |
| #`                         |               | Implemented in [mruby-io][mruby-io]. |
| #abort                     |       o       |
| #exec                      |       o       |
| #exit                      |       o       |
| #exit!                     |       o       |
| #fork                      |       o       | If fork is not usable, Process.respond_to?(:fork) returns false. |
| #sleep                     |               | Implemented in [mruby-sleep][mruby-sleep]. |
| #spawn                     |       o       |
| #system                    |               |


### Signal

- https://ruby-doc.org/core-2.4.1/Signal.html

| method                     | mruby-process | Comment |
| -------------------------  | :-----------: | :-----  |
| ::signame                  |       o       |
| ::list                     |       o       |
| ::trap                     |               | Implemented in [mruby-signal][mruby-signal]. |


## Development

Clone the repo:
    
    $ git clone https://github.com/appplant/mruby-process.git && cd mruby-process/

Compile the source:

    $ rake compile

Run the tests:

    $ rake test


## Caveats

 - $? may not work correctly on the platform where ``pid_t`` is not ``int`` or
   ``MRB_INT_MAX`` is less than ``PID_MAX`` (or /proc/sys/kernel/pid_max).


## License

Copyright (c) 2012 Internet Initiative Japan Inc.
Copyright (c) 2017 appPlant GmbH.

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


[mruby]: https://github.com/mruby/mruby
[mruby-process-ext]: https://github.com/ksss/mruby-process-ext
[mruby-process-sys]: https://github.com/haconiwa/mruby-process-sys
[mruby-sleep]: https://github.com/matsumotory/mruby-sleep
[mruby-io]: https://github.com/iij/mruby-io
[mruby-signal]: https://github.com/ksss/mruby-signal
[process_h]: https://github.com/appPlant/mruby-process/blob/windows/include/process.h
