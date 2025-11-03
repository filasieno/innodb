##
## XInnoDB Nix Flake
##
## - Dev shells for Clang and GCC (mutually exclusive toolchains)
## - Checks to build and run tests under both toolchains (for `nix flake check`)
## - Uses CMake presets (see CMakePresets.json) for consistent configure/build/test
##
{
  description = "XInnoDB embedded storage engine";

  # Inputs: pinned nixpkgs for modern compilers/tools
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  # Outputs: packages, devShells, checks, formatter
  outputs =
    { self, nixpkgs }:
    let
      # Supported systems
      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
      forAllSystems = nixpkgs.lib.genAttrs systems;

      # Common build configuration shared between packages and devShells
      commonConfig = system: {
        pkgs = nixpkgs.legacyPackages.${system};
      };

      # Build dependencies - tools needed to compile the project
      buildDeps =
        pkgs: with pkgs; [
          git
          cmake
          ninja
          doxygen
          bison
          flex
          pkg-config
          gtest
          gbenchmark
          graphviz
          python3
          gcovr
          lcov
          fmt_12.dev
          liburing.dev
        ];

      # Runtime dependencies - libraries needed at runtime
      runtimeDeps =
        { pkgs }: with pkgs; [
          liburing
          fmt_12
        ];

      # Development tools - additional tools for development environment
      devTools =
        pkgs: with pkgs; [
          cmakeWithGui
          ccache
          luajit
          sysbench
          bun
        ];

      # Common dev shell (toolchain provided via stdenv; extras via extraPkgs)
      mkCommonDevShell = system: stdenv: extraPkgs: devShellEx:
        let
          config = commonConfig system;
          pkgs = config.pkgs;
        in
        stdenv.mkDerivation {
          name = "xinnodb-devshell";

          nativeBuildInputs = buildDeps pkgs ++ devTools pkgs ++ extraPkgs ++ [ stdenv.cc ];
          buildInputs = runtimeDeps config;

          # Configure PKG_CONFIG_PATH for proper library discovery
          shellHook = ''
            export PKG_CONFIG_PATH="${
              pkgs.lib.concatStringsSep ":" [
                (pkgs.lib.makeSearchPathOutput "dev" "lib/pkgconfig" (
                  runtimeDeps config ++ buildDeps pkgs ++ [ stdenv.cc ]
                ))
                (pkgs.lib.makeSearchPathOutput "dev" "share/pkgconfig" (
                  runtimeDeps config ++ buildDeps pkgs ++ [ stdenv.cc ]
                ))
                (pkgs.lib.makeSearchPath "lib/pkgconfig"   (runtimeDeps config ++ buildDeps pkgs ++ [ stdenv.cc ]))
                (pkgs.lib.makeSearchPath "share/pkgconfig" (runtimeDeps config ++ buildDeps pkgs ++ [ stdenv.cc ]))
              ]
            }:$PKG_CONFIG_PATH"

            # Make GTest discoverable by CMake
            export CMAKE_PREFIX_PATH="${pkgs.gtest}:${pkgs.fmt_12}:$CMAKE_PREFIX_PATH"

            # Project-specific environment setup
            export PROJECT_ROOT=$(git rev-parse --show-toplevel 2>/dev/null || pwd)
            export PATH="$PROJECT_ROOT/build/tests/bin:$PROJECT_ROOT/build/unit_tests/bin:$PROJECT_ROOT/scripts:$PATH"

            # Toolchain-specific env (injected by caller)
            ${devShellEx}

            # ccache: speed up rebuilds
            export CCACHE_DIR="''${XDG_CACHE_HOME:-$HOME/.cache}/ccache/xinnodb"
            mkdir -p "$CCACHE_DIR"
            export CCACHE_MAXSIZE=10G
            export CCACHE_COMPRESS=1
            export CCACHE_BASEDIR="$PROJECT_ROOT"
            # Make CMake use ccache automatically
            export CMAKE_C_COMPILER_LAUNCHER=ccache
            export CMAKE_CXX_COMPILER_LAUNCHER=ccache

            # Custom prompt
            export PS1="\[\e[1;33m\](xinnodb)\[\e[0m\] \[\e[1;32m\][\u@\h:\w]\$\[\e[0m\] "

            echo "Using packages: "
            echo "  compiler     : $CC ($CXX) [${stdenv.cc}]"
            echo "  cmake        : ${pkgs.cmake}"
            echo "  ninja        : ${pkgs.ninja}"
            echo "  gcovr        : ${pkgs.gcovr}"
            echo "  lcov         : ${pkgs.lcov}"
            echo "  gtest        : ${pkgs.gtest}"
            echo "  gbenchmark   : ${pkgs.gbenchmark}"
            echo "  fmt.dev      : ${pkgs.fmt_12.dev}"
            echo "  liburing.dev : ${pkgs.liburing.dev}"
          '';

          phases = [ "installPhase" ];
          installPhase = "mkdir -p $out";
        };

    in
    {
      # Packages: build and install static library + public headers
      packages = forAllSystems (system: {
        default =
          let
            config = commonConfig system;
            pkgs = config.pkgs;
          in
          pkgs.stdenv.mkDerivation {
            pname = "xinnodb";
            version = "0.1.0";
            # Include untracked files too: filter explicitly with builtins.filterSource
            src = builtins.filterSource (
              path: type:
                let rel = pkgs.lib.removePrefix (toString ./. + "/") (toString path);
                in (
                  false
                  || rel == "CMakeLists.txt"
                  || rel == "CMakePresets.json"
                  || rel == "xinnodb" 
                  || pkgs.lib.hasPrefix "xinnodb/" rel
                )
            ) ./.;

            outputs = [
              "out"
              "dev"
            ];

            nativeBuildInputs     = buildDeps pkgs ++ [ pkgs.stdenv.cc pkgs.ccache ];
            buildInputs           = runtimeDeps config;
            propagatedBuildInputs = [ pkgs.liburing ];

            doCheck = true;

            configurePhase = ''
              echo "Configuring XInnoDB..."
              find . -type f -print | sort
              echo "--- Module src files ---"
              find xinnodb/src -type f -print | sort || true
              export CCACHE_DIR="''${XDG_CACHE_HOME:-$HOME/.cache}/ccache/xinnodb"
              mkdir -p "$CCACHE_DIR"
              export CCACHE_MAXSIZE=10G
              export CCACHE_COMPRESS=1
              export CCACHE_BASEDIR="$PWD"
              cmake -S . -B build/release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
              echo "✅ Configuration completed"
            '';

            buildPhase = ''
              echo "Building XInnoDB..."
              cmake --build build/release -j
              cmake --build build/release --target doc -j
              echo "✅ Build completed"
            '';

            checkPhase = ''
              echo "Checking XInnoDB..."
              cmake --build build/release --target xinnodb_tests -j
              ctest --test-dir build/release --output-on-failure -j
              echo "✅ Tests passed"
            '';

            # Install shared/runtime bits to $out, static libs and headers to $dev
            installPhase = ''
              echo "Installing XInnoDB..."  
              cmake --install build/release --component runtime      --prefix $out
              cmake --install build/release --component dev          --prefix $dev
              echo "✅ Installation complete"
            '';

            meta = with pkgs.lib; {
              description = "XInnoDB embedded database engine";
              homepage = "https://github.com/sunbains/embedded-innodb";
              license = licenses.gpl2Only;
              platforms = platforms.linux;
              maintainers = [
                "Fabio N. Filasieno"
                "Roberto Boati"
              ];
            };
          };
       
      });

      # Checks (CI-like): build and run tests under both toolchains using presets
      checks = forAllSystems (
        system:
        let
          config = commonConfig system;
          pkgs = config.pkgs;
        in
        {
          # Clang check: select clang/clang++ explicitly and use `debug` preset
          clang = pkgs.clangStdenv.mkDerivation {
            name = "xinnodb-check-clang";
            src = ./.;
            nativeBuildInputs = buildDeps pkgs ++ [ pkgs.clang pkgs.ninja pkgs.cmake pkgs.ccache ];
            buildInputs = runtimeDeps config;
            doCheck = true;
            configurePhase = ''
              export CC=clang
              export CXX=clang++
              export CCACHE_DIR="''${XDG_CACHE_HOME:-$HOME/.cache}/ccache/xinnodb"
              mkdir -p "$CCACHE_DIR"
              export CCACHE_MAXSIZE=10G
              export CCACHE_COMPRESS=1
              export CCACHE_BASEDIR="$PWD"
              export CMAKE_C_COMPILER_LAUNCHER=ccache
              export CMAKE_CXX_COMPILER_LAUNCHER=ccache
              cmake --preset debug
            '';
            buildPhase = ''
              cmake --build --preset debug -j
            '';
            checkPhase = ''
              ctest --preset debug --output-on-failure -j
            '';
            installPhase = ''
              mkdir -p $out
            '';
          };

          # GCC check: select gcc/g++ explicitly and use `debug` preset
          gcc = pkgs.stdenv.mkDerivation {
            name = "xinnodb-check-gcc";
            src = ./.;
            nativeBuildInputs = buildDeps pkgs ++ [ pkgs.gcc pkgs.ninja pkgs.cmake pkgs.ccache ];
            buildInputs = runtimeDeps config;
            doCheck = true;
            configurePhase = ''
              export CC=gcc
              export CXX=g++
              export CCACHE_DIR="''${XDG_CACHE_HOME:-$HOME/.cache}/ccache/xinnodb"
              mkdir -p "$CCACHE_DIR"
              export CCACHE_MAXSIZE=10G
              export CCACHE_COMPRESS=1
              export CCACHE_BASEDIR="$PWD"
              export CMAKE_C_COMPILER_LAUNCHER=ccache
              export CMAKE_CXX_COMPILER_LAUNCHER=ccache
              cmake --preset debug
            '';
            buildPhase = ''
              cmake --build --preset debug -j
            '';
            checkPhase = ''
              ctest --preset debug --output-on-failure -j
            '';
            installPhase = ''
              mkdir -p $out
            '';
          };
        }
      );

      # Developer shells: mutually exclusive toolchains (default: clang)
      devShells = forAllSystems (
        system:
        let
          pkgs = import nixpkgs {
            inherit system;
            overlays = [ ];
          };
        in
        {
          default = self.devShells.${system}.clang;

          # GCC dev shell: include coverage tools; avoid any clang tools here
          gcc = mkCommonDevShell system pkgs.stdenv (
            with pkgs; [ gcovr lcov ]
          ) ''
            export CC=gcc
            export CXX=g++
            export CMAKE_C_COMPILER=$CC
            export CMAKE_CXX_COMPILER=$CXX

          '';

          # Clang dev shell: include clang-tools/llvm/lld; avoid any gcc tools here
          clang = mkCommonDevShell system pkgs.clangStdenv (
            with pkgs; [ clang-tools llvmPackages.llvm llvmPackages.lld ]
          ) ''
            export CC=clang
            export CXX=clang++
            export CMAKE_C_COMPILER=$CC
            export CMAKE_CXX_COMPILER=$CXX
          '';
        }
      );

      # Formatter: consistent Nix formatting across systems
      formatter = forAllSystems (system: nixpkgs.legacyPackages.${system}.nixpkgs-fmt);
    };
}
