mruby-process
=========

## install by mrbgems
 - add conf.gem line to `build_config.rb`
```ruby
MRuby::Build.new do |conf|

    # ... (snip) ...

    conf.gem :git => 'https://github.com/iij/mruby-process.git'
end
```

## Features

 - Proccess ::waitpid, ::pid, ::kill, ::fork, ::ppid

## License
This software is licensed under the same license terms of the original mruby.

