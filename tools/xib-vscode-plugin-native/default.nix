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

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
    "-DNODE_INCLUDE_DIR=${nodejs}/include/node"
  ];

  installPhase = ''
    runHook preInstall
    cmake --install build
    runHook postInstall
  '';

  meta = with lib; {
    description = "Standalone Node-API native addon for the XIB VSCode plugin";
    license = licenses.mit;
    platforms = platforms.linux;
  };
}