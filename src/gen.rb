#!/usr/bin/env ruby

Dir.chdir(File.dirname($0))

f = File.open("signals.cstub", "w")

IO.readlines("signals.def").each { |name|
  name.sub(/^#.*/, "")
  name.strip!
  next if name.empty?

  raise "invalid signal name: #{name}" unless name =~ /^SIG(.+)/
  sym = $1

  f.write <<CODE
#ifdef #{name}
  { "#{sym}", #{name} },
#endif
CODE
}
