{ pkgs ? import <nixpkgs> {} }:

pkgs.stdenv.mkDerivation {
  pname = "innodb-formats";
  version = "0.1.0";
  
  src = ./.;
  
  nativeBuildInputs = with pkgs; [
    kaitai-struct-compiler
  ];
  
  buildPhase = ''
    echo "Compiling InnoDB Kaitai Struct formats..."
    
    # Create output directory
    mkdir -p compiled
    
    # Compile all .ksy files to Python (for validation)
    echo "Validating and compiling formats..."
    
    # Compile root files
    for ksy in *.ksy; do
      echo "  Compiling $ksy..."
      kaitai-struct-compiler --target python --outdir compiled "$ksy" || {
        echo "ERROR: Failed to compile $ksy"
        exit 1
      }
    done
    
    # Compile page type files in subdirectories
    for dir in system btree undo blob containers; do
      if [ -d "$dir" ]; then
        echo "Compiling $dir/*.ksy..."
        for ksy in "$dir"/*.ksy; do
          if [ -f "$ksy" ]; then
            echo "  Compiling $ksy..."
            kaitai-struct-compiler --target python --outdir "compiled/$dir" "$ksy" || {
              echo "ERROR: Failed to compile $ksy"
              exit 1
            }
          fi
        done
      fi
    done
    
    echo "✅ All formats compiled successfully!"
  '';
  
  installPhase = ''
    mkdir -p $out
    
    # Install source .ksy files
    cp -r *.ksy $out/
    cp -r system btree undo blob containers $out/
    cp README.md $out/
    
    # Install compiled Python modules
    mkdir -p $out/compiled
    cp -r compiled/* $out/compiled/
    
    echo "✅ Installation complete"
  '';
  
  meta = with pkgs.lib; {
    description = "InnoDB file format specifications in Kaitai Struct";
    license = licenses.mit;
    platforms = platforms.all;
    maintainers = [
      "Fabio N. Filasieno"
    ];
  };
}

