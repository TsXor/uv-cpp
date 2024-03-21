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
            and fn_type.params[1] == KW_TO_P_REQUEST_TYPES[cb_kw] \
            or fn_type.params[0] == KW_TO_P_REQUEST_TYPES[cb_kw]
    except (IndexError, KeyError):
        return False

def check_async_api(fn_type: CTFunction, fn_name: str, cb_kw: str):
    fail_info = f'Function {fn_name} cannot be identified as a {cb_kw} callback.'
    
    fn_namesp = f'uv_{cb_kw}_'
    if not fn_name.startswith(fn_namesp):
        raise ValueError(fail_info)
    fn_clean_name = fn_name[len(fn_namesp):]
    if not check_async_api_heading_param(fn_type, cb_kw):
        raise ValueError(fail_info)
    return fn_clean_name
