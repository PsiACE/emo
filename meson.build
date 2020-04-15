project('emo', 'c',
  version : '0.1.0',
  meson_version: '>= 0.50.0',
  default_options : ['warning_level=3'])


config_h = configuration_data()
config_h.set_quoted('PACKAGE_VERSION', meson.project_version())
config_h.set('DEBUG', false)

configure_file(
  output: 'emo-config.h',
  configuration: config_h,
)
add_project_arguments([
  '-I' + meson.build_root(),
], language: 'c')

subdir('include')
subdir('src')

declare_dependency(include_directories : incdir)

run_target(
  'format',
  command: ['scripts/format.sh']
)

run_target(
  'release',
  command: ['scripts/release.sh', meson.project_name(), meson.project_version()]
)

run_target(
  'run',
  command: ['scripts/run.sh']
)
