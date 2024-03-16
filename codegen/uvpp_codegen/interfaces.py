import abc
from pycparser import c_ast
from .ctype_utils import *


class INodeHandler(abc.ABC):
    @abc.abstractmethod
    def interested_node_types(self) -> list[type[c_ast.Node]]:
        raise NotImplementedError

    @abc.abstractmethod
    def handle(self, node: c_ast.Node, typedefs: dict[str, 'c_ast.Declarator']) -> bool:
        raise NotImplementedError
