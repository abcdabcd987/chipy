project('chipy', 'cpp', default_options: ['cpp_std=c++1z'])

inc_dirs = include_directories(get_option('prefix') + '/include', '/usr/local/include', './include/')
prefix_library_path=[get_option('prefix')+'/lib/x86_64-linux-gnu', get_option('prefix')+'/lib', '/usr/local/lib', '/usr/local/lib/x86_64-linux-gnu']

subdir('./src/compiler')
subdir('./src/interpreter')
subdir('./test')

gtest = dependency('gtest', main : true)
gmock = dependency('gmock')

cpp = meson.get_compiler('cpp')
log_dep = cpp.find_library('glog')
json_dep = cpp.find_library('document', dirs: prefix_library_path)
pypa_dep = cpp.find_library('pypa', dirs: prefix_library_path)

chipy = shared_library('chipy', [compiler_cpp_files, interpreter_cpp_files], include_directories: inc_dirs, dependencies: [log_dep, json_dep, pypa_dep], install: true)

sgx_sdk_dir = '/opt/intel/sgxsdk'
sgx_library_path = sgx_sdk_dir + '/lib64'

sgx_tlibc_incdir = include_directories(sgx_sdk_dir+'/include/tlibc')
sgx_stdcxx_incdir = include_directories(sgx_sdk_dir+'/include/libcxx')
sgx_incdir = include_directories(sgx_sdk_dir + '/include')
sgx_trts_dep = cpp.find_library('sgx_trts', dirs: sgx_library_path)

cpp = meson.get_compiler('cpp')

sgx_tstdc_dep = cpp.find_library('sgx_tstdc', dirs: sgx_library_path)
sgx_tstdcxx_dep = cpp.find_library('sgx_tcxx', dirs: sgx_library_path)
json_sgx_dep = cpp.find_library('document_sgx', dirs: prefix_library_path)

sgx_compile_args = ['-DIS_ENCLAVE', '-nostdinc', '-fstack-protector', '-pie', '-UNDEBUG', '-UEDEBUG']

chipy_sgx = static_library('chipy_sgx', interpreter_cpp_files,
    c_args: sgx_compile_args,
    cpp_args: ['-std=c++11', '-nostdinc++'] + sgx_compile_args,
    link_args:['-Wl,--no-undefined','-nostdlib','-nodefaultlibs','-nostartfiles','-L'+sgx_sdk_dir+'/lib64',
    '-Wl,--whole-archive','-lsgx_trts','-Wl,--no-whole-archive',
    '-Wl,--start-group','-lsgx_tstdc','-lsgx_tcxx', '-lsgx_tkey_exchange','-ldocument_sgx','-Wl,--end-group',
    ],
    include_directories: [sgx_incdir, sgx_tlibc_incdir, sgx_stdcxx_incdir, inc_dirs], install: true)


install_subdir('include/chipy', install_dir : 'include')

executable('chipy-test', test_cpp_files, dependencies: [gtest, json_dep, pypa_dep], link_with: chipy, include_directories: inc_dirs)
