incdir = include_directories('.')

cli_headers = [
    'cli/args.h',
    'cli/messages.h',
    'cli/styles.h',
]

core_headers = [
    'core/chunk.h',
    'core/common.h',
    'core/debug.h',
    'core/memory.h',
    'core/value.h',
]

include_headers = [
    'emo.h'
]

emo_headers = files(cli_headers, core_headers, core_headers)

install_headers(include_headers, subdir: 'emo')
