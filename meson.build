project('chipy', 'cpp', default_options: ['cpp_std=c++1z'])

inc_dirs = include_directories('./include')

subdir('./src/compiler')
subdir('./src/interpreter')
subdir('./test')

gtest = dependency('gtest', main : true)
gmock = dependency('gmock')

cpp = meson.get_compiler('cpp')
log_dep = cpp.find_library('glog')
json_dep = cpp.find_library('document')
pypa_dep = cpp.find_library('pypa')

chipy = shared_library('chipy', [compiler_cpp_files, interpreter_cpp_files], include_directories: inc_dirs, dependencies: [log_dep, json_dep, pypa_dep], install: true)

install_subdir('include/chipy', install_dir : 'include')

executable('chipy-test', test_cpp_files, dependencies: [gtest], link_with: chipy, include_directories: inc_dirs)
