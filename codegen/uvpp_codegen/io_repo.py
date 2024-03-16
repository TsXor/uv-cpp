from typing import Iterable
from io import StringIO
from pathlib import Path
from .utils import *


class IORepo: # class as a singleton
    existing: dict[tuple[str, ...], StringIO] = {}

    @classmethod
    def get_io(cls, *relpth: str):
        io = dict_get_or_default(cls.existing, relpth, StringIO)
        return io
    
    @classmethod
    def dump_all(cls, pth: Path):
        assert pth.is_dir()
        for relpth, sio in cls.existing.items():
            real_pth = path_join_list(pth, relpth)
            real_pth.parent.mkdir(exist_ok=True, parents=True)
            real_pth.write_text(sio.getvalue(), encoding='utf-8')
