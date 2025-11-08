{ lib
, stdenv
, cmake
, ninja
, pkg-config
, tree-sitter
, gtest
, tree-sitter-cli ? null
}:

stdenv.mkDerivation rec {
  pname = "tree-sitter-cps";
  version = "0.1.0";

  src = ./.;

  outputs = [ "out" "dev" ];

  nativeBuildInputs = [
    cmake
    ninja
    pkg-config
  ] ++ lib.optional (tree-sitter-cli != null) tree-sitter-cli;

  buildInputs = [
    tree-sitter
  ];

  nativeCheckInputs = [
    gtest
  ];

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
    "-DBUILD_TESTING=ON"
  ];

  # Generate parser.c from grammar.json during build
  preConfigure = lib.optionalString (tree-sitter-cli != null) ''
    echo "Generating parser.c from grammar.json..."
    tree-sitter generate src/grammar.json --abi=15
  '';

  doCheck = true;

  checkPhase = ''
    runHook preCheck

    echo "Running CPS parser tests..."
    ./cps_parser_test

    runHook postCheck
  '';

  # Install both static and shared libraries to dev output
  # Headers and pkg-config to dev output
  # Runtime libraries to out output
  postInstall = ''
    mkdir -p $dev/lib/pkgconfig
    mv $out/lib/pkgconfig/tree-sitter-cps.pc $dev/lib/pkgconfig/ 2>/dev/null || true
  '';

  meta = with lib; {
    description = "CPS grammar for tree-sitter";
    homepage = "https://github.com/tree-sitter/tree-sitter-cps";
    license = licenses.mit;
    platforms = platforms.linux;
    maintainers = [
      "Fabio N. Filasieno"
    ];
  };
}
