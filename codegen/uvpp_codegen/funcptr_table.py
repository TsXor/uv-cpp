'''
Record unordered_map-s for identifying function pointer
in exception info.
'''

from .io_repo import IORepo
from .utils import *
from .ctype_utils import *
from .libuv_conventions import *

table_element_tmpl = '{{(void*){fn_name}, "{fn_name}"}},\n'

def process_information(fn_type: CTFunction, cb_kw: str, fn_clean_name: str, param_names: list[str]):
    fn_name = f'uv_{cb_kw}_{fn_clean_name}'
    ele = table_element_tmpl.format(fn_name=fn_name)
    IORepo.get_io('funcptr_table', f'{cb_kw}.hpp').write(ele)
