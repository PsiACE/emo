incdir = include_directories('.')

shared_headers = [
    'emo/emo.h'
]

utils_headers = [
    'emo/utils/args.h',
    'emo/utils/messages.h',
    'emo/utils/styles.h',
]

emo_headers = files(shared_headers, utils_headers)

install_headers(shared_headers, subdir: 'emo')
