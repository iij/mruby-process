MRuby::Gem::Specification.new('mruby-process') do |spec|
  spec.license = 'MIT'
  spec.authors = 'IIJ'

  spec.cc.include_paths << "#{build.root}/src"
end
