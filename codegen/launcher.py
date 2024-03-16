import sys, runpy
from datetime import datetime
from pathlib import Path

WHERE_AM_I = Path(__file__).parent
LAUNCHER_GLOBAL_NAME = '__launcher_config__'
MODULE_NAME = 'uvpp_codegen'

if __name__ == '__main__':
    laucher_config = {}
    runpy.run_path(
        str(WHERE_AM_I / MODULE_NAME),
        {LAUNCHER_GLOBAL_NAME: laucher_config},
        '__main__',
    )
