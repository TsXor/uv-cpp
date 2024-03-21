from pycparser import c_ast
from .interfaces import INodeHandler
from .ctype_utils import *
from .libuv_conventions import *
from . import callback_wrap
from . import funcptr_table


class UVFuncHandler(INodeHandler):
    def interested_node_types(self) -> list[type[c_ast.Node]]:
        return [c_ast.Decl]
    
    def handle(self, node: c_ast.Decl, typedefs: dict[str, 'c_ast.Declarator']) -> bool:
        node_type = node.type
        if type(node_type) == c_ast.FuncDecl:
            return self.real_handle(node_type, node.name)
        else:
            return False
    
    def real_handle(self, node: c_ast.FuncDecl, fn_name: str) -> bool:
        if not fn_name.startswith('uv_'): return False
        fn_type = treeify_declarator_type(node)
        param_names = [] if node.args is None else \
            [decl.name for decl in node.args.params if type(decl) == c_ast.Decl]
        if callback_wrap.is_async_api(fn_type):
            cb_kw = CALLBACK_TYPES_TO_KW[fn_type.params[-1]] # type: ignore
            try:
                fn_clean_name = check_async_api(fn_type, fn_name, cb_kw)
                callback_wrap.process_information(fn_type, cb_kw, fn_clean_name, param_names)
                funcptr_table.process_information(fn_type, cb_kw, fn_clean_name, param_names)
            except ValueError as verr: 
                print(f'Ignored: {verr}')
        return True
    