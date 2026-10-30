#!/usr/bin/env python3
"""xref.py: find every access to every struct field, with its exact struct type.

    python3 tools/session/xref.py                    build .session/xref.json from all of src/
    python3 tools/session/xref.py report STRUCT      summary of each field of STRUCT (reads, writes, values)
    python3 tools/session/xref.py field STRUCT FIELD every access to one field, with the source line

Each source file is preprocessed with line markers and parsed with pycparser. Member accesses (a.b, p->b)
are typed by following declarations. Fields that share a name across structs (unk1C and so on) are kept
apart. The JSON keeps each access's file, line, column, function, kind (read, write, update, address taken),
the value written and the constants it is compared with. rename.py uses the same data.
"""
import collections, json, os, re, subprocess, sys

GW = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
from pycparser import c_ast, c_generator, c_parser

OUT = os.environ.get('XREF_JSON') or os.path.join(GW, '.session', 'xref.json')
CPP = [os.path.join(GW, 'tools/gcc-2.7.2/gcc'), '-E', '-Iinclude', '-Iinclude/PR', '-Isrc', '-I.', '-D_LANGUAGE_C',
       '-DF3DEX_GBI', '-D_FINALROM', '-DNDEBUG', '-DTARGET_N64', '-D__attribute__(x)=', '-D__extension__=']
GEN = c_generator.CGenerator()


def preprocess(path):
    env = dict(os.environ, COMPILER_PATH=os.path.join(GW, 'tools/gcc-2.7.2'))
    text = subprocess.run(CPP + [path], cwd=GW, capture_output=True, text=True, env=env, check=True).stdout
    # INCLUDE_ASM/INCLUDE_RODATA become top-level __asm__ statements, which pycparser does not know
    # (and #ident/#pragma lines, which it does not accept either)
    text = re.sub(r'^(__asm__\(|#ident|#pragma).*$', '', text, flags=re.M)
    # GCC extensions pycparser lacks: case ranges, and string literals that run over a line break (joined,
    # with an empty line left behind; line numbers stay the same)
    text = re.sub(r'\bcase\s+([^:\n]+?)\s*\.\.\.\s*[^:\n]+:', r'case \1:', text)
    lines = text.split('\n')
    for i in range(len(lines) - 1):
        if len(re.findall(r'(?<!\\)"', re.sub(r"'(\\.|[^'])'", '', lines[i]))) % 2:
            lines[i] += '\\n' + lines[i + 1]
            lines[i + 1] = ''
    return '\n'.join(lines)


class Types:
    """Struct layouts and typedefs collected from the parsed files."""

    def __init__(self):
        self.typedefs = {}   # name -> type node
        self.structs = {}    # struct key -> {'fields': {name: type node}, 'decl': {name: (file, line)}, 'kind'}
        self.tag_to_key = {}
        self.node_key = {}   # id(struct node) -> key (pycparser nodes have __slots__)

    def key_for(self, node, hint=None):
        """A stable name for a struct or union node: its typedef name, else its tag, else HINT."""
        if node.decls is None:
            return self.tag_to_key.get(node.name, node.name)
        return self.node_key.get(id(node)) or hint or node.name

    def add_struct(self, node, key):
        if node.decls is None or key is None:
            return
        self.node_key[id(node)] = key
        if node.name:
            self.tag_to_key.setdefault(node.name, key)
        if key in self.structs and self.structs[key]['fields']:
            return
        fields, decl = {}, {}
        for d in node.decls:
            if d.name is None:
                continue
            fields[d.name] = d.type
            decl[d.name] = (d.coord.file, d.coord.line)
            inner = strip_decl(d.type)
            if isinstance(inner, (c_ast.Struct, c_ast.Union)) and inner.decls is not None:
                self.add_struct(inner, inner.name and self.tag_to_key.get(inner.name) or f'{key}.{d.name}')
        self.structs[key] = {'fields': fields, 'decl': decl, 'kind': type(node).__name__.lower()}

    def struct_of(self, t):
        """The struct key of type T (after typedefs), or None."""
        t = self.resolve(t)
        if isinstance(t, (c_ast.Struct, c_ast.Union)):
            return self.key_for(t)
        return None

    def resolve(self, t, depth=0):
        """Strip TypeDecl layers and expand typedef names; stop at a pointer, array, function or struct."""
        while depth < 50:
            depth += 1
            if isinstance(t, c_ast.TypeDecl):
                t = t.type
            elif isinstance(t, c_ast.IdentifierType):
                name = t.names[-1]
                if name in self.typedefs:
                    t = self.typedefs[name]
                else:
                    return t
            elif isinstance(t, c_ast.Typename):
                t = t.type
            else:
                return t
        return t


def strip_decl(t):
    while isinstance(t, (c_ast.TypeDecl, c_ast.PtrDecl, c_ast.ArrayDecl)):
        t = t.type
    return t


def target(t, types):
    """What a pointer or array points to."""
    t = types.resolve(t)
    if isinstance(t, (c_ast.PtrDecl, c_ast.ArrayDecl)):
        return t.type
    return None


class Walker(c_ast.NodeVisitor):
    def __init__(self, types, src, out):
        self.types, self.src, self.out = types, src, out
        self.globals = {}
        self.funcs = {}
        self.scopes = []
        self.func = None
        self.consts = []      # numeric constants with their context, for enumify.py
        self.switches = []    # text of the enclosing switch expressions

    # declarations
    def declare(self, decl):
        if decl.name is None:
            return
        (self.scopes[-1] if self.scopes else self.globals)[decl.name] = decl.type
        if isinstance(decl.type, c_ast.FuncDecl):
            self.funcs[decl.name] = decl.type.type

    def lookup(self, name):
        for s in reversed(self.scopes):
            if name in s:
                return s[name]
        return self.globals.get(name)

    def visit_Typedef(self, node):
        self.types.typedefs[node.name] = node.type
        inner = strip_decl(node.type)
        if isinstance(inner, (c_ast.Struct, c_ast.Union)):
            self.types.add_struct(inner, node.name if isinstance(node.type, c_ast.TypeDecl) else None)

    def visit_Decl(self, node):
        inner = strip_decl(node.type)
        if isinstance(inner, (c_ast.Struct, c_ast.Union)) and inner.decls is not None:
            self.types.add_struct(inner, inner.name)
        if isinstance(node.type, c_ast.FuncDecl):
            self.funcs[node.name] = node.type.type
            self.globals.setdefault(node.name, node.type)
            return
        self.declare(node)
        if node.init is not None:
            self.const(node.init, 'set', node.name)
            self.expr(node.init)

    def visit_FuncDef(self, node):
        self.funcs[node.decl.name] = node.decl.type.type
        self.globals[node.decl.name] = node.decl.type
        self.func = node.decl.name
        self.scopes = [{}]
        args = node.decl.type.args
        if args:
            for p in args.params:
                if isinstance(p, c_ast.Decl):
                    self.declare(p)
        self.stmt(node.body)
        self.scopes = []
        self.func = None

    # statements
    def stmt(self, n):
        if n is None:
            return
        if isinstance(n, c_ast.Compound):
            self.scopes.append({})
            for item in n.block_items or []:
                self.stmt(item)
            self.scopes.pop()
        elif isinstance(n, c_ast.Decl):
            self.visit_Decl(n)
        elif isinstance(n, c_ast.DeclList):
            for d in n.decls:
                self.visit_Decl(d)
        elif isinstance(n, (c_ast.If, c_ast.TernaryOp)):
            self.expr(n.cond, 'cond')
            self.stmt(n.iftrue)
            self.stmt(n.iffalse)
        elif isinstance(n, (c_ast.While, c_ast.DoWhile)):
            self.expr(n.cond, 'cond')
            self.stmt(n.stmt)
        elif isinstance(n, c_ast.For):
            self.scopes.append({})
            self.stmt(n.init) if isinstance(n.init, (c_ast.DeclList, c_ast.Decl)) else self.expr(n.init)
            self.expr(n.cond, 'cond')
            self.expr(n.next)
            self.stmt(n.stmt)
            self.scopes.pop()
        elif isinstance(n, c_ast.Switch):
            self.expr(n.cond, 'switch')
            self.switches.append(GEN.visit(n.cond))
            self.stmt(n.stmt)
            self.switches.pop()
        elif isinstance(n, (c_ast.Case,)):
            if self.switches:
                self.const(n.expr, 'case', self.switches[-1])
            for s in n.stmts or []:
                self.stmt(s)
        elif isinstance(n, c_ast.Default):
            for s in n.stmts or []:
                self.stmt(s)
        elif isinstance(n, c_ast.Label):
            self.stmt(n.stmt)
        elif isinstance(n, c_ast.Return):
            self.const(n.expr, 'return', self.func or '')
            self.expr(n.expr)
        elif isinstance(n, (c_ast.Goto, c_ast.Break, c_ast.Continue, c_ast.EmptyStatement, c_ast.Pragma)):
            pass
        elif isinstance(n, c_ast.Typedef):
            self.visit_Typedef(n)
        else:
            self.expr(n)

    def const(self, n, kind, where):
        """Record N if it is a plain integer constant (for enumify.py)."""
        if isinstance(n, c_ast.Constant) and n.type in ('int', 'unsigned int', 'long int') and n.coord:
            self.consts.append({'file': n.coord.file, 'line': n.coord.line, 'col': n.coord.column, 'text': n.value,
                                'kind': kind, 'where': where, 'func': self.func})

    # expressions: return the type node of N (or None), recording member accesses on the way
    def expr(self, n, ctx='read', extra=None):
        if n is None:
            return None
        T = self.types
        if isinstance(n, c_ast.ID):
            return self.lookup(n.name)
        if isinstance(n, c_ast.Constant):
            return None
        if isinstance(n, c_ast.StructRef):
            base = self.expr(n.name, 'base')
            if n.type == '->':
                base = target(base, T) if base is not None else None
            key = T.struct_of(base) if base is not None else None
            fname = n.field.name
            ftype = None
            if key and key in T.structs:
                ftype = T.structs[key]['fields'].get(fname)
            self.out.append({'struct': key, 'field': fname, 'file': n.field.coord.file, 'line': n.field.coord.line,
                             'col': n.field.coord.column, 'func': self.func, 'kind': ctx,
                             **(extra or {})})
            return ftype
        if isinstance(n, c_ast.ArrayRef):
            self.const(n.subscript, 'index', GEN.visit(n.name))
            self.expr(n.subscript)
            t = self.expr(n.name, 'base' if ctx in ('write', 'update', 'addr') else ctx)
            return target(t, T) if t is not None else None
        if isinstance(n, c_ast.UnaryOp):
            if n.op == '&':
                t = self.expr(n.expr, 'addr')
                return c_ast.PtrDecl([], t) if t is not None else None
            if n.op == '*':
                t = self.expr(n.expr, ctx if ctx != 'base' else 'read')
                return target(t, T) if t is not None else None
            if n.op in ('++', '--', 'p++', 'p--'):
                return self.expr(n.expr, 'update')
            if n.op == 'sizeof':
                self.expr(n.expr, 'sizeof') if not isinstance(n.expr, c_ast.Typename) else None
                return None
            if n.op == '!':
                self.expr(n.expr, 'cond' if ctx == 'cond' else 'read')
                return None
            return self.expr(n.expr, 'read')
        if isinstance(n, c_ast.Assignment):
            self.const(n.rvalue, 'set', GEN.visit(n.lvalue))
            rhs = GEN.visit(n.rvalue)
            self.expr(n.rvalue)
            if n.op == '=':
                return self.expr(n.lvalue, 'write', {'value': rhs[:80]})
            return self.expr(n.lvalue, 'update', {'value': f'{n.op} {rhs[:76]}'})
        if isinstance(n, c_ast.BinaryOp):
            if n.op in ('==', '!=', '<', '>', '<=', '>=', '&') and (isinstance(n.right, c_ast.Constant)
                                                                     or isinstance(n.left, c_ast.Constant)):
                const = n.right if isinstance(n.right, c_ast.Constant) else n.left
                other = n.left if const is n.right else n.right
                self.const(const, 'cmp' + n.op, GEN.visit(other))
                self.expr(other, 'cmp', {'value': f'{n.op} {const.value}'})
                return None
            lt = self.expr(n.left, 'cond' if n.op in ('&&', '||') and ctx == 'cond' else 'read')
            rt = self.expr(n.right, 'cond' if n.op in ('&&', '||') and ctx == 'cond' else 'read')
            if n.op in ('+', '-'):
                for t in (lt, rt):
                    if t is not None and isinstance(T.resolve(t), (c_ast.PtrDecl, c_ast.ArrayDecl)):
                        return c_ast.PtrDecl([], target(t, T))
            return lt if n.op not in ('==', '!=', '<', '>', '<=', '>=', '&&', '||') else None
        if isinstance(n, c_ast.Cast):
            self.expr(n.expr, ctx if ctx in ('base', 'addr') else 'read')
            return n.to_type.type
        if isinstance(n, c_ast.TernaryOp):
            self.expr(n.cond, 'cond')
            a = self.expr(n.iftrue)
            b = self.expr(n.iffalse)
            return a or b
        if isinstance(n, c_ast.FuncCall):
            if n.args:
                first = GEN.visit(n.args.exprs[0])
                for k, a in enumerate(n.args.exprs):
                    self.const(a, f'arg{k}', GEN.visit(n.name) + '|' + first)
                    self.expr(a, 'arg', {'value': GEN.visit(n.name)[:40]})
            if isinstance(n.name, c_ast.ID):
                return self.funcs.get(n.name.name)
            t = self.expr(n.name)
            t = T.resolve(t) if t is not None else None
            if isinstance(t, c_ast.PtrDecl):
                t = T.resolve(t.type)
            return t.type if isinstance(t, c_ast.FuncDecl) else None
        if isinstance(n, c_ast.ExprList):
            t = None
            for e in n.exprs:
                t = self.expr(e)
            return t
        if isinstance(n, c_ast.InitList):
            for e in n.exprs:
                self.expr(e)
            return None
        if isinstance(n, c_ast.CompoundLiteral):
            self.expr(n.init)
            return n.type.type
        if isinstance(n, (c_ast.Compound, c_ast.If, c_ast.Return, c_ast.Decl, c_ast.DeclList)):
            self.stmt(n)
            return None
        return None


def build():
    types = Types()
    accesses = []
    consts = []
    files = sorted(f for f in os.listdir(os.path.join(GW, 'src')) if f.endswith('.c'))
    failed = []
    for f in files:
        try:
            ast = c_parser.CParser().parse(preprocess(os.path.join('src', f)), f)
        except Exception as e:
            failed.append((f, str(e)[:200]))
            continue
        w = Walker(types, f, accesses)
        for ext in ast.ext:
            w.visit(ext) if not isinstance(ext, c_ast.FuncDef) else w.visit_FuncDef(ext)
        consts += w.consts
    structs = {k: {'kind': v['kind'], 'fields': {fn: {'type': GEN.visit(ft) if not isinstance(ft, c_ast.TypeDecl)
                                                        else ' '.join(getattr(ft.type, 'names', [])) or type(ft.type).__name__,
                                                        'decl': v['decl'][fn]}
                                                   for fn, ft in v['fields'].items()}}
               for k, v in types.structs.items()}
    data = {'structs': structs, 'accesses': [a for a in accesses if a['struct']],
            'untyped': [a for a in accesses if not a['struct']], 'failed': failed, 'consts': consts}
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    json.dump(data, open(OUT, 'w'))
    print(f"{len(data['accesses'])} typed accesses, {len(data['untyped'])} untyped, {len(structs)} structs, "
          f"failed files: {[f for f, _ in failed]}")
    for f, e in failed:
        print('  ', f, e)


def load():
    if not os.path.exists(OUT):
        build()
    return json.load(open(OUT))


def report(struct):
    d = load()
    s = d['structs'].get(struct)
    if not s:
        sys.exit(f'no struct {struct}')
    acc = collections.defaultdict(list)
    for a in d['accesses']:
        if a['struct'] == struct:
            acc[a['field']].append(a)
    for fname, info in s['fields'].items():
        al = acc.get(fname, [])
        kinds = collections.Counter(a['kind'] for a in al)
        funcs = collections.Counter(a['func'] for a in al)
        writes = collections.Counter(a.get('value', '++/--') for a in al if a['kind'] in ('write', 'update'))
        cmps = collections.Counter(a.get('value') for a in al if a['kind'] == 'cmp')
        args = collections.Counter(a.get('value') for a in al if a['kind'] == 'arg')
        print(f"{fname} ({info['type']}): " + ', '.join(f'{k} {v}' for k, v in kinds.most_common()))
        if al:
            print('  functions: ' + ', '.join(f'{f} {n}' for f, n in funcs.most_common(8)))
        if writes:
            print('  written: ' + '; '.join(f'{v} x{n}' for v, n in writes.most_common(8)))
        if cmps:
            print('  compared: ' + '; '.join(f'{v} x{n}' for v, n in cmps.most_common(8)))
        if args:
            print('  passed to: ' + '; '.join(f'{v} x{n}' for v, n in args.most_common(6)))


def field(struct, fname):
    d = load()
    lines = {}
    for a in d['accesses']:
        if a['struct'] == struct and a['field'] == fname:
            path = os.path.join(GW, 'src', a['file']) if not a['file'].startswith(('include', 'src')) else os.path.join(GW, a['file'])
            if path not in lines:
                lines[path] = open(path).read().split('\n') if os.path.exists(path) else []
            text = lines[path][a['line'] - 1].strip() if a['line'] <= len(lines[path]) else ''
            print(f"{a['file']}:{a['line']} {a['func']} [{a['kind']}] {text[:150]}")


if __name__ == '__main__':
    if len(sys.argv) == 1:
        build()
    elif sys.argv[1] == 'report':
        report(sys.argv[2])
    elif sys.argv[1] == 'field':
        field(sys.argv[2], sys.argv[3])
