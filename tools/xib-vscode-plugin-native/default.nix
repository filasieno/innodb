{ lib
, stdenv
, nodejs
, cmake
, ninja
}:

stdenv.mkDerivation rec {
  pname = "xib-vscode-plugin-native";
  version = "0.0.1";

  src = ./.;

  nativeBuildInputs = [ cmake ninja ];

  buildInputs = [ nodejs ];

  buildPhase = ''
    runHook preBuild
    export NODE_INCLUDE_DIR="${nodejs}/include/node"
    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DNODE_INCLUDE_DIR="$NODE_INCLUDE_DIR"
    cmake --build build --config Release -j
    runHook postBuild
  '';

  installPhase = ''
    runHook preInstall
    mkdir -p "$out/lib"
    cp build/addon.node "$out/lib/"
    runHook postInstall
  '';

  meta = with lib; {
    description = "Standalone Node-API native addon for the XIB VSCode plugin";
    license = licenses.mit;
    platforms = platforms.linux;
  };
}