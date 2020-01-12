project('CSHook', 'cpp',
    version: '0.0.1',
    license: 'MIT',
    default_options: 'cpp_std=c++17'
)

cpptoml_inc = include_directories('dependencies/cpptoml/include')
cpptoml = declare_dependency(include_directories: cpptoml_inc)

glad_inc = include_directories('dependencies/glad/include')
glad = declare_dependency(include_directories: glad_inc)

cxx = meson.get_compiler('cpp')
dwmapi = cxx.find_library('dwmapi')
psapi = cxx.find_library('psapi')

subdir('src')