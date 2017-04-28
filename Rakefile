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

MRUBY_CONFIG  = File.expand_path(ENV['MRUBY_CONFIG'] || 'build_config.rb')
MRUBY_VERSION = ENV['MRUBY_VERSION'] || 'head'

def mtask(cmd)
  if Gem.win_platform?
    Dir.chdir('mruby') do
      sh "set MRUBY_CONFIG=#{MRUBY_CONFIG} && ruby .\\minirake #{cmd}"
    end
  else
    sh "cd mruby && MRUBY_CONFIG=#{MRUBY_CONFIG} ./minirake #{cmd}"
  end
end

file :mruby do
  if MRUBY_VERSION == 'head'
    sh 'git clone --depth 1 git://github.com/mruby/mruby.git'
  else
    sh "curl -L --fail --retry 3 --retry-delay 1 https://github.com/mruby/mruby/archive/#{MRUBY_VERSION}.tar.gz -s -o - | tar zxf -"
    mv "mruby-#{MRUBY_VERSION}", 'mruby'
  end
end

desc 'compile binary'
task compile: :mruby do
  mtask 'all --verbose'
end

desc 'test'
task test: :mruby do
  mtask 'test'
end

desc 'cleanup'
task :clean do
  mtask 'deep_clean'
end
