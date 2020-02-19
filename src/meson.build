shared_sources = [
    'main.c'
]

utils_sources = [
    'utils/args.c',
    'utils/messages.c',
]

emo_sources = files(shared_sources, utils_sources)

emo_deps = []

executable('emo', emo_sources,
  include_directories : incdir,
  dependencies: emo_deps,
  install: true,
)
