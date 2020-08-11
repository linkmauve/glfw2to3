project('glfw2to3', 'c',
  version: '2.7.10',
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

libglfw = shared_library('glfw',
  sources,
  include_directories: includes,
  dependencies: [dl, pthread],
  version: '2.7.10',
  soversion: '2',
  install: true,
)

install_headers('include/GL/glfw.h', subdir : 'GL')

pkg = import('pkgconfig')
pkg.generate(
  libraries: [libglfw],
  version: '2.7.10',
  name: 'libglfw',
  filebase: 'libglfw',
  description: 'Porting library to make GLFW 2.x games run on top of GLFW 3.x',
)
