MRuby::Gem::Specification.new('mruby-process') do |spec|
  spec.license = 'MIT'
  spec.authors = 'mruby developers'

  spec.mruby_includes = ["#{build.root}/src"]
end
