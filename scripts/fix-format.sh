#!/usr/bin/env bash
find src/ -iname *.h -o -iname *.cc -o -iname *.h.in | xargs clang-format -i -style=file -fallback-style=LLVM -assume-filename=../.clang-format
