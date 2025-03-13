project('baremetal_async',
  languages : ['cpp'],
  version : '0.1.0',
  license: 'BSL-1.0',
  default_options: ['cpp_std=c++20']
  )

boost_dep = dependency('boost', required: true)

baremetal_concurrency_sub = subproject('cpp-baremetal-concurrency')
baremetal_concurrency_dep = baremetal_concurrency_sub.get_variable('concurrency_dep')

stdx_sub = subproject('cpp-std-extensions')
stdx_dep = stdx_sub.get_variable('stdx_dep')

async_dep = declare_dependency(
  include_directories: inc,
  dependencies: [baremetal_concurrency_dep, boost_dep, stdx_dep]
  )
