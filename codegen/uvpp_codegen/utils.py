from typing import Iterable, TypeVar, Callable
from pathlib import Path

K = TypeVar('K'); V = TypeVar('V')
def dict_get_or_default(dic: dict[K, V], key: K, default_sup: Callable[[], V]) -> V:
    if key in dic:
        return dic[key]
    else:
        default = default_sup()
        dic[key] = default
        return default

def path_join_list(pth: Path, relpth: Iterable[str]):
    result = pth
    for comp in relpth:
        result = result / comp
    return result
