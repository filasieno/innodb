#!/bin/bash

echo "=== Generating C++ code for all InnoDB Kaitai Struct files ==="
echo "Target: ../build/innodb-formats/cpp"
echo "Note: Using cpp_stl target (Kaitai Struct only provides STL-based C++ generation)"
echo

# Create the target directory
mkdir -p ../build/innodb-formats/cpp

find . -name "*.ksy" | sort | while read -r file; do
    echo "Generating C++: $file"
    if nix develop /home/fabio/code/innodb --command kaitai-struct-compiler --target cpp_stl --outdir ../build/innodb-formats/cpp "$file" 2>&1 | grep -q "error"; then
        echo "❌ FAILED to generate C++ for $file"
        exit 1
    else
        echo "✅ Generated"
    fi
    echo
done

echo "🎉 All C++ files generated successfully!"
echo "Generated $(find ../build/innodb-formats/cpp -name "*.cpp" -o -name "*.h" | wc -l) C++ files"
