"""Repair root-plus's classic-dictionary .rootmap files after install.

usage: fix_rootmaps.py <prefix>

Only needed where root-plus builds classic dictionaries (macOS, and where
ROOT has no runtime C++ modules, e.g. conda-forge's linux-aarch64). Two
things rootcling writes there break ROOT's autoloading and every macro that
includes a root-plus header:

1. Forward declarations of templates over an enum ("template <Clock> class
   TClock;", "template <ROOT::IOPlus::Plane> class TBiquadFilter;") come
   without a declaration of the enum, so cling rejects the whole block.
   A scoped enum can be declared opaquely, so declare each one first.
2. Headers of the core library are recorded by bare name (TKeyboard.h),
   because that library also has include/ROOT/IOPlus/Core on its include
   path; ROOT then cannot find them. Use the unique path under include/.
"""
import pathlib
import re
import sys

prefix = pathlib.Path(sys.argv[1])
include = prefix / "include"
headers = list((include / "ROOT" / "IOPlus").rglob("*.h"))

# every scoped enum root-plus defines: name -> True
scoped = set()
for h in headers:
    scoped.update(re.findall(r"\benum\s+class\s+(\w+)\b", h.read_text(errors="ignore")))


def opaque(qualified):
    """'ROOT::IOPlus::Plane' -> 'namespace ROOT { namespace IOPlus { enum class Plane; } }'"""
    *ns, name = qualified.split("::")
    decl = f"enum class {name};"
    for n in reversed(ns):
        decl = f"namespace {n} {{ {decl} }}"
    return decl


for rootmap in sorted(prefix.glob("lib/libRIOPlus*.rootmap")):
    lines = rootmap.read_text().split("\n")
    out, added = [], set()
    nttp = sorted({q for q in re.findall(r"template <([\w:]+)>", "\n".join(lines))
                   if q.split("::")[-1] in scoped})
    for line in lines:
        m = re.match(r"^header ([^/\s]+)$", line)
        if m:
            found = [h for h in headers if h.name == m.group(1)]
            if len(found) == 1:
                line = f"header {found[0].relative_to(include)}"
        out.append(line)
        if line.strip() == "{ decls }":
            for q in nttp:
                out.append(opaque(q))
                added.add(q)
    rootmap.write_text("\n".join(out))
    bare = [l for l in out if re.match(r"^header [^/\s]+$", l)]
    print(f"{rootmap.name}: declared {sorted(added) or 'no'} enums; "
          f"{len(bare)} bare header(s) left {[l.split()[1] for l in bare]}")
