project('glfw2to3', 'c',
  version: '0.1.0',
  meson_version: '>=0.54',
  default_options: [
    'warning_level=3',
    'c_std=gnu11',
  ],
  license: 'Zlib',
)

add_global_arguments('-Wno-pedantic', language: 'c')

sources = [
  'src/enable.c',
  'src/extension.c',
  'src/image.c',
  'src/init.c',
  'src/input.c',
  'src/joystick.c',
  'src/threading.c',
  'src/time.c',
  'src/video.c',
  'src/window.c',
]

includes = include_directories('include')

cc = meson.get_compiler('c')
dl = cc.find_library('dl')
pthread = cc.find_library('pthread')

shared_library('glfw',
  sources,
  include_directories: includes,
  dependencies: [dl, pthread],
  version : '2.7.10',
  soversion : '2'
)
