bin_directory = meson.source_root() + '/bin/'
source_files = []

subdir('Features')
subdir('HLSDK')
subdir('Hooks')
subdir('ImGui')

shared_library('cshook', source_files + ['main.cpp'],
    install: true,
    install_dir: bin_directory,
    dependencies: [
        cpptoml,
        glad,
        dwmapi,
        psapi
    ],
    cpp_args: ['-DUNICODE', '-D_UNICODE', '-DNOMINMAX']
)