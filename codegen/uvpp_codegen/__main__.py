import subprocess
import sys
import textwrap
from pathlib import Path

from pycparser import c_parser
from uvpp_codegen import AllinNodeVisitor, UVFuncHandler, IORepo


MODULE_DIR = Path(__file__).parent


def is_empty_or_directive(line: str):
    line = line.strip()
    if not line: return True
    return line.startswith('#')

def preprocess_code(code_path: Path):
    fake_libc_dir = str(MODULE_DIR / 'pycparser_fake_libc')
    incl_dir = str(code_path.absolute().parent)
    raw_code = code_path.read_text(encoding='utf-8')
    header = textwrap.dedent('''
        #define __attribute__(...)
        #define __inline__
        #define __inline
        #define __asm__(...)
        #define __volatile__(...)
        #define __extension__
        #define __v2df int
        #define __v2di int
        #define __v4df int
        #define __v4sf int
        #define __v4si int
        #define __v8sf int
        #define __v8hi int
        #define __v16qi int
    ''')
    raw_code = header + raw_code
    sp = subprocess.run(['cpp', f'-I{incl_dir}', f'-I{fake_libc_dir}'],
                        input=raw_code.encode(), capture_output=True, cwd=incl_dir)
    if sp.returncode:
        raise ValueError(f'proprocessing with cpp failed: {sp.stderr.decode()}')
    code = sp.stdout.decode()
    code = code.replace('__builtin_offsetof', 'offsetof')
    code_lines = code.replace('\r', '').split('\n')
    return '\n'.join(filter(lambda l: not is_empty_or_directive(l), code_lines))


if __name__ == '__main__':
    uv_header_path = Path(sys.argv[1])
    output_home = Path(sys.argv[2])
    
    code = preprocess_code(uv_header_path)
    parser = c_parser.CParser()
    #(Path(__file__).parent / 'prep.h').write_text(code)
    ast = parser.parse(code)

    visitor = AllinNodeVisitor()
    visitor.add_handler(UVFuncHandler())
    visitor.visit(ast)

    IORepo.dump_all(output_home)
