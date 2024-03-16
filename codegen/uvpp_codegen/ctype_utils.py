from typing import cast, overload, Literal
from dataclasses import dataclass
from pycparser import c_ast
from pycparser.c_generator import CGenerator


TypeSpecifierCode = Literal['S', 'U', 'E', 'I']
TYPE_SPECIFIER_CODE_MAP: dict[type['c_ast.TypeSpecifier'], TypeSpecifierCode] = {
    c_ast.Struct: 'S',
    c_ast.Union: 'U',
    c_ast.Enum: 'E',
    c_ast.IdentifierType: 'I',
}

TypeQualifierCode = Literal['C', 'V', 'R']
TYPE_QUALIFIER_CODE_MAP: dict['c_ast.TypeQualifier', TypeQualifierCode] = {
    'const': 'C',
    'volatile': 'V',
    'restrict': 'R',
}
TYPE_QUALIFIER_CODE_REV_MAP: dict[TypeQualifierCode, 'c_ast.TypeQualifier'] = {
    'C': 'const',
    'V': 'volatile',
    'R': 'restrict',
}


@dataclass(frozen=True)
class CTPointer:
    child: 'CTTypeNode'
    def __repr__(self) -> str:
        return f'P({repr(self.child)})'

@dataclass(frozen=True)
class CTArray:
    child: 'CTTypeNode'
    dim: str
    def __repr__(self) -> str:
        return f'A({repr(self.child)}, {repr(self.dim)})'

@dataclass(frozen=True)
class CTFunction:
    ret: 'CTTypeNode'
    params: tuple['CTTypeNode', ...]
    have_vararg: bool
    def __repr__(self) -> str:
        code = 'FV' if self.have_vararg else 'F'
        return f'{code}({repr(self.ret)}, {repr(self.params)})'

@dataclass(frozen=True)
class CTSymbol:
    spec: TypeSpecifierCode
    type: str
    def __repr__(self) -> str:
        return f'{self.spec}({repr(self.type)})'

@dataclass(frozen=True)
class CTQualifier:
    child: 'CTTypeNode'
    quals: frozenset[TypeQualifierCode]
    def __repr__(self) -> str:
        qual_str = ''.join(sorted(self.quals))
        return f'Q({repr(self.child)}, {repr(qual_str)})'

CTTypeNode = CTPointer | CTArray | CTFunction | CTSymbol | CTQualifier


@overload
def treeify_declarator_type(decl: c_ast.PtrDecl) -> CTPointer: ...
@overload
def treeify_declarator_type(decl: c_ast.ArrayDecl) -> CTArray: ...
@overload
def treeify_declarator_type(decl: c_ast.FuncDecl) -> CTFunction: ...
@overload
def treeify_declarator_type(decl: c_ast.TypeDecl) -> CTSymbol: ...

def treeify_declarator_type(decl: 'c_ast.Declarator') -> CTTypeNode:
    '''太棒了，我逐渐理解一切。'''
    if type(decl) == c_ast.TypeDecl:
        specifier_type = TYPE_SPECIFIER_CODE_MAP[type(decl.type)]
        if type(decl.type) == c_ast.IdentifierType:
            type_name = ' '.join(decl.type.names)
        else:
            type_name = cast(str, decl.type.name) # type: ignore
        result = CTSymbol(specifier_type, type_name)
        if decl.quals:
            qual_set = frozenset(TYPE_QUALIFIER_CODE_MAP[qual] for qual in decl.quals)
            result = CTQualifier(result, qual_set)
        return result
    elif type(decl) == c_ast.PtrDecl:
        result = CTPointer(treeify_declarator_type(decl.type))
        if decl.quals:
            qual_set = frozenset(TYPE_QUALIFIER_CODE_MAP[qual] for qual in decl.quals)
            result = CTQualifier(result, qual_set)
        return result
    elif type(decl) == c_ast.ArrayDecl:
        dim_expr = '' if decl.dim is None else CGenerator().visit(decl.dim)
        return CTArray(treeify_declarator_type(decl.type), dim_expr)
    elif type(decl) == c_ast.FuncDecl:
        if decl.args is None:
            have_vararg = False
            params = []
        elif type(decl.args.params[-1]) == c_ast.EllipsisParam:
            have_vararg = True
            params = decl.args.params[:-1]
        else:
            have_vararg = False
            params = decl.args.params
        params = cast(list[c_ast.Decl], params)
        return CTFunction(treeify_declarator_type(decl.type),
                          tuple(treeify_declarator_type(pd.type) for pd in params), have_vararg)
    else:
        raise ValueError('not a declarator!')


def eval_ctype_repr(typ: str) -> CTTypeNode:
    '''recover a CTTypeNode from repr string'''
    def fn_normal(ret: CTTypeNode, params: tuple[CTTypeNode]):
        return CTFunction(ret, params, False)
    def fn_vararg(ret: CTTypeNode, params: tuple[CTTypeNode]):
        return CTFunction(ret, params, True)
    def struct(typ: str): return CTSymbol('S', typ)
    def union(typ: str): return CTSymbol('U', typ)
    def enum(typ: str): return CTSymbol('E', typ)
    def identifier(typ: str): return CTSymbol('I', typ)
    def qualifier(ct: CTTypeNode, quals: str):
        qual_set = frozenset(q for q in quals if q in {'C', 'V', 'R'})
        return CTQualifier(ct, cast(frozenset[TypeQualifierCode], qual_set))
    return eval(typ, {
        'P': CTPointer,
        'A': CTArray,
        'F': fn_normal,
        'FV': fn_vararg,
        'S': struct,
        'U': union,
        'E': enum,
        'I': identifier,
        'Q': qualifier,
    })


def format_ctype_tree(ct: CTTypeNode, name: str = '') -> str:
    '''format C type decl of a CTTypeNode'''
    return _format_ctype_subtree(ct, name)

def _format_ctype_subtree(ct: CTTypeNode, name: str) -> str:
    decl = name
    last_is_ptr = False

    while True:
        if type(ct) == CTPointer:
            decl = '*' + decl
            last_is_ptr = True
            ct = ct.child
        elif type(ct) == CTArray:
            if last_is_ptr: decl = f'({decl})'
            decl = decl + f'[{ct.dim}]'
            last_is_ptr = False
            ct = ct.child
        elif type(ct) == CTSymbol:
            prefix_map: dict[TypeSpecifierCode, str] = {
                'S': 'struct ',
                'U': 'union ',
                'E': 'enum ',
                'I': '',
            }
            typename = prefix_map[ct.spec] + ct.type
            return typename + ' ' + decl
        elif type(ct) == CTFunction:
            if last_is_ptr: decl = f'({decl})'
            param_types = [_format_ctype_subtree(pt, '').rstrip() for pt in ct.params]
            param_exp = f'({", ".join(param_types)})'
            decl = decl + param_exp
            last_is_ptr = False
            ct = ct.ret
        elif type(ct) == CTQualifier:
            qual_prefix = ' '.join(TYPE_QUALIFIER_CODE_REV_MAP[q] for q in ct.quals)
            decl = qual_prefix + ' ' + decl
            ct = ct.child
        else:
            raise ValueError('this shouldn\'t happen')
