#!/bin/bash

echo "=== Validating all InnoDB Kaitai Struct files ==="
echo

find . -name "*.ksy" | sort | while read -r file; do
    echo "Validating: $file"
    if nix develop /home/fabio/code/innodb --command kaitai-struct-compiler --target cpp_stl --outdir /tmp/kaitai_test "$file" 2>&1; then
        echo "✅ OK"
    else
        echo "❌ FAILED"
        exit 1
    fi
    echo
done

echo "🎉 All files validated successfully!"
