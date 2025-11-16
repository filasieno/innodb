{ lib
, stdenv
, nodejs
, cmake
, ninja
}:

stdenv.mkDerivation rec {
  pname = "xinnodb-node-addon";
  version = "0.0.1";

  src = ./.;

  nativeBuildInputs = [ cmake ninja ];

  buildInputs = [ nodejs ];

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
    "-DNODE_INCLUDE_DIR=${nodejs}/include/node"
  ];

# doCheck = true;

# checkPhase = ''
#   runHook preCheck
#   ctest --test-dir build --output-on-failure -j
#   runHook postCheck
# '';

  installPhase = ''
    runHook preInstall
    mkdir -p $out/lib
    cp ib_node_plugin.node $out/lib/
    runHook postInstall
  '';

  meta = with lib; {
    description = "Standalone Node-API native addon for XInnoDB";
    license = licenses.mit;
    platforms = platforms.linux;
  };
}