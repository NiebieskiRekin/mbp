project('miniproj2', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++17', 'optimization=3']
)

benchmark_dep = dependency('benchmark')

executable('benchmark_reordering',
           'benchmark_reordering.cpp',
           dependencies: benchmark_dep,
           link_args: ['-static-libstdc++'])

executable('benchmark_reordering_separated',
           'benchmark_reordering_separated.cpp',
           dependencies: benchmark_dep,
           link_args: ['-static-libstdc++'])