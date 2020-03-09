cli_sources = [
    'cli/args.c',
    'cli/main.c',
    'cli/messages.c',
]

core_sources = [
    'core/chunk.c',
    'core/debug.c',
    'core/memory.c',
    'core/value.c',
    'core/vm.c',
]

emo_sources = files(cli_sources, core_sources)

emo_deps = []

executable('emo', emo_sources,
  include_directories : incdir,
  dependencies: emo_deps,
  install: true,
)
