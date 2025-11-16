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

  configureFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
    "-DNODE_INCLUDE_DIR=${nodejs}/include/node"
  ];

  buildPhase = ''
    runHook preBuild
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