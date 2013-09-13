MRuby::Gem::Specification.new('mruby-process-win32') do |spec|
  # based on https://github.com/iij/mruby-process
  spec.license = 'MIT'
  spec.authors = 'Internet Initiative Japan Inc.'

  spec.cc.include_paths << "#{build.root}/src"
end
