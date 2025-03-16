project('baremetal_async',
  ['cpp'],
  version : '0.1.0',
  license: 'BSL-1.0',
  default_options: ['cpp_std=c++20']
  )

boostmp11_sub = subproject('boostmp11')
boostmp11_dep = boostmp11_sub.get_variable('boostmp11_dep')

baremetal_concurrency_sub = subproject('cpp-baremetal-concurrency')
baremetal_concurrency_dep = baremetal_concurrency_sub.get_variable('baremetal_concurrency_dep')

stdx_sub = subproject('cpp-std-extensions')
stdx_dep = stdx_sub.get_variable('stdx_dep')

inc = include_directories('include')

async_dep = declare_dependency(
  include_directories: inc,
  dependencies: [baremetal_concurrency_dep, boostmp11_dep, stdx_dep]
  )
