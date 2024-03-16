from pycparser import c_ast
from .interfaces import INodeHandler
from .ctype_utils import *


class AllinNodeVisitor(c_ast.NodeVisitor):
    handlers: dict[type[c_ast.Node], list[INodeHandler]]
    known_typedefs: dict[str, 'c_ast.Declarator']

    def __init__(self) -> None:
        self.handlers = {}
        self.known_typedefs = {}

    def add_handler(self, cg: INodeHandler):
        subscr_types = cg.interested_node_types()
        for typ in subscr_types:
            if typ in self.handlers:
                self.handlers[typ].append(cg)
            else:
                self.handlers[typ] = [cg]

    def visit(self, node: c_ast.Node):
        """ Delete method routing, we don't need it. """
        return self.generic_visit(node)

    def generic_visit(self, node: c_ast.Node):
        if type(node) != c_ast.Typedef:
            handled = False
            for cg in self.handlers.get(type(node), []):
                handled = cg.handle(node, self.known_typedefs)
            if not handled:
                super().generic_visit(node)
        else:
            self.known_typedefs[node.name] = node.type
