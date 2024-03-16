'''
Most common things shou be auto-generated with template,
here we have boilerplate callback wrappers.
'''

import textwrap
from .io_repo import IORepo
from .utils import *
from .ctype_utils import *


CALLBACK_KEYWORDS = (
    'alloc',
    'read',
    'write',
    'connect',
    'shutdown',
    'connection',
    'close',
    'poll',
    'timer',
    'async',
    'prepare',
    'check',
    'idle',
    'exit',
    'walk',
    'fs',
    'work',
    'after_work',
    'getaddrinfo',
    'getnameinfo',
    'random',
)

CALLBACK_TYPES_TO_KW = {
    CTSymbol('I', f'uv_{cb_kw}_cb'): cb_kw
    for cb_kw in CALLBACK_KEYWORDS
}

KW_TO_P_REQUEST_TYPES = {
    cb_kw: CTPointer(CTSymbol('I', f'uv_{cb_kw}_t'))
    for cb_kw in CALLBACK_KEYWORDS
}

P_UV_LOOP_T = CTPointer(CTSymbol('I', 'uv_loop_t'))


def is_async_api(fn_type: CTFunction):
    return fn_type.params[-1] in CALLBACK_TYPES_TO_KW

def check_async_api_heading_param(fn_type: CTFunction, cb_kw: str):
    try:
        return fn_type.params[0] == P_UV_LOOP_T \
            and fn_type.params[1] == KW_TO_P_REQUEST_TYPES[cb_kw]
    except (IndexError, KeyError):
        return False

def process_information(fn_type: CTFunction, fn_name: str, param_names: list[str]):
    # different callbacks may have different traits
    # so just handle them differently
    cb_kw = CALLBACK_TYPES_TO_KW[fn_type.params[-1]] # type: ignore
    handler_name = f'generate_{cb_kw}_callback'
    if handler_name in globals():
        globals()[handler_name](fn_type, fn_name, param_names)


# coroutine api
fs_coroutine_api_tmpl = textwrap.dedent('''
    UVPP_FN auto {gen_name}(uv_loop_t* loop, uv_fs_t* req, {real_param_typed}) {{
        return coro_wrap(uv_fs_{gen_name}, loop, req, {real_param_notype});
    }}
''')

def generate_fs_callback(fn_type: CTFunction, fn_name: str, param_names: list[str]):
    def warn_special():
        print('fs callback: '
              f'Caution! Function {fn_name} may be s special callback! '
              'It is now ignored but you should check it.')
    
    fn_namesp = 'uv_fs_'
    if not fn_name.startswith(fn_namesp):
        warn_special(); return
    gen_name = fn_name[len(fn_namesp):]
    if not check_async_api_heading_param(fn_type, 'fs'):
        warn_special(); return
    
    real_param_typed: list[str] = []
    real_param_notype: list[str] = []
    for param_name, param_type in zip(param_names, fn_type.params):
        real_param_notype.append(param_name)
        real_param_typed.append(format_ctype_tree(param_type, param_name))
    
    format_args = dict[str, str](
        gen_name = gen_name,
        real_param_notype = ', '.join(real_param_notype[2:-1]),
        real_param_typed = ', '.join(real_param_typed[2:-1]),
    )

    fs_coroutine_api = fs_coroutine_api_tmpl.format(**format_args)

    IORepo.get_io('fs', 'coroutine.hpp').write(fs_coroutine_api)

