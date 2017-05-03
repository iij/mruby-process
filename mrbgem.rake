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

MRuby::Gem::Specification.new('mruby-process') do |spec|
  spec.license = 'MIT'
  spec.authors = 'mruby developers'

  spec.add_test_dependency 'mruby-print',   core: 'mruby-print'
  spec.add_test_dependency 'mruby-time',    core: 'mruby-time'
  spec.add_test_dependency 'mruby-os',      mgem: 'mruby-os'
  spec.add_test_dependency 'mruby-tiny-io', mgem: 'mruby-tiny-io'

  def target_win32?
    return true if ENV['OS'] == 'Windows_NT'
    build.is_a?(MRuby::CrossBuild) && build.host_target.include?('mingw')
  end

  spec.cc.defines << 'HAVE_MRB_PROCESS_H'
  spec.objs.delete objfile("#{build_dir}/src/win32") unless target_win32?
end
