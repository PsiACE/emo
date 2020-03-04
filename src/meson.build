cli_sources = [
    'cli/args.c',
    'cli/messages.c',
    'cli/main.c'
]

emo_sources = files(cli_sources)

emo_deps = []

executable('emo', emo_sources,
  include_directories : incdir,
  dependencies: emo_deps,
  install: true,
)
