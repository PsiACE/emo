incdir = include_directories('.')

include_headers = [
    'emo/include/emo.h'
]

cli_headers = [
    'emo/cli/args.h',
    'emo/cli/messages.h',
    'emo/cli/styles.h',
]

emo_headers = files(include_headers, cli_headers)

install_headers(include_headers, subdir: 'emo')
