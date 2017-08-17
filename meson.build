project('chipy', 'cpp', default_options: ['cpp_std=c++1z'])

inc_dirs = include_directories('./include', '/usr/local/include')

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

sgx_sdk_dir = '/opt/intel/sgxsdk'
sgx_library_path = sgx_sdk_dir + '/lib64'

sgx_tlibc_incdir = include_directories(sgx_sdk_dir+'/include/tlibc')
sgx_stdcxx_incdir = include_directories(sgx_sdk_dir+'/include/libcxx')
sgx_incdir = include_directories(sgx_sdk_dir + '/include')

cpp = meson.get_compiler('cpp')

sgx_tstdc_dep = cpp.find_library('sgx_tstdc', dirs: sgx_library_path)
sgx_tstdcxx_dep = cpp.find_library('sgx_tcxx', dirs: sgx_library_path)

chipy_sgx = static_library('chipy_sgx', [interpreter_cpp_files], cpp_args: ['-nostdinc', '-DIS_ENCLAVE'], include_directories: [sgx_incdir, sgx_tlibc_incdir, sgx_stdcxx_incdir, inc_dirs], dependencies: [log_dep, json_dep, pypa_dep], install: true, dependencies: [sgx_tstdc_dep, sgx_tstdcxx_dep])

install_subdir('include/chipy', install_dir : 'include')

executable('chipy-test', test_cpp_files, dependencies: [gtest], link_with: chipy, include_directories: inc_dirs)
