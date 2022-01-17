#!/usr/bin/env bash
find src/ -iname *.h -o -iname *.cc -o -iname *.h.in | xargs clang-format --dry-run -Werror -style=file -fallback-style=LLVM
