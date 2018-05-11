#!/usr/bin/env ruby
#
# mrbgems test runner
#

gemname = File.basename(File.dirname(File.expand_path __FILE__))

ENV['MRUBY_VERSION'] ||= 'master'

if __FILE__ == $0
  url, dir = 'https://github.com/mruby/mruby', 'tmp/mruby'
  build_args = ARGV
  build_args = ['all', 'test']  if build_args.nil? or build_args.empty?

  Dir.mkdir 'tmp'  unless File.exist?('tmp')
  unless File.exist?(dir)
    if ENV['MRUBY_VERSION'] == 'master'
      system "git clone --depth 1 #{url}.git #{dir}"
    else
      system "curl -sfL #{url}/archive/#{ENV['MRUBY_VERSION']}.tar.gz | tar zxf -"
      system "mv mruby-#{ENV['MRUBY_VERSION']} #{dir}"
    end
  end

  exit system(%Q[cd #{dir}; MRUBY_CONFIG=#{File.expand_path __FILE__} ruby minirake #{build_args.join(' ')}])
end

MRuby::Build.new do |conf|
  toolchain :gcc
  conf.gembox 'default'
  conf.enable_test

  conf.gem File.expand_path(File.dirname(__FILE__))
end
