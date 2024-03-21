'''
Most common things should be auto-generated with template,
here we have boilerplate callback wrappers.
'''

import textwrap
from .io_repo import IORepo
from .utils import *
from .ctype_utils import *
from .libuv_conventions import *


def process_information(fn_type: CTFunction, cb_kw: str, fn_clean_name: str, param_names: list[str]):
    # different callbacks may have different traits
    # so just handle them differently
    handler_name = f'generate_{cb_kw}_callback'
    if handler_name in globals():
        globals()[handler_name](fn_type, fn_clean_name, param_names)


# coroutine api
fs_coroutine_api_tmpl = textwrap.dedent('''
    UVPP_FN auto {gen_name}(uv_loop_t* loop, uv_fs_t* req, {real_param_typed}) {{
        return coro_wrap(uv_fs_{gen_name}, loop, req, {real_param_notype});
    }}
''')

def generate_fs_callback(fn_type: CTFunction, fn_clean_name: str, param_names: list[str]):
    real_param_typed: list[str] = []
    real_param_notype: list[str] = []
    for param_name, param_type in zip(param_names, fn_type.params):
        real_param_notype.append(param_name)
        real_param_typed.append(format_ctype_tree(param_type, param_name))
    
    format_args = dict[str, str](
        gen_name = fn_clean_name,
        real_param_notype = ', '.join(real_param_notype[2:-1]),
        real_param_typed = ', '.join(real_param_typed[2:-1]),
    )

    fs_coroutine_api = fs_coroutine_api_tmpl.format(**format_args)

    IORepo.get_io('fs', 'coroutine.hpp').write(fs_coroutine_api)

