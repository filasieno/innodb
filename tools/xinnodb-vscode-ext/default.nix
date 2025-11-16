{ lib
, stdenv
, nodejs
, esbuild
}:

stdenv.mkDerivation rec {
  pname = "xinnodb-vscode-ext";
  version = "0.0.1";

  src = ./.;

  nativeBuildInputs = [ nodejs esbuild ];

  # Build: transpile TS with esbuild (no network)
  buildPhase = ''
    runHook preBuild

    mkdir -p dist
    esbuild src/extension.ts src/interfaces.ts --bundle --platform=node --format=cjs --external:vscode --outfile=dist/extension.js

    # Compile tests if they exist
    if [ -d test ]; then
      mkdir -p out/test
      esbuild test/**/*.ts --bundle --platform=node --format=cjs --external:vscode --outdir=out/test
    fi

    runHook postBuild
  '';

  installPhase = ''
    runHook preInstall
    mkdir -p "$out"
    cp -r . "$out"/
    runHook postInstall
  '';

  meta = with lib; {
    description = "XInnoDB VS Code extension (Hello World) for the XInnoDB project";
    homepage = "https://github.com/sunbains/embedded-innodb";
    license = licenses.mit;
    platforms = platforms.linux;
    maintainers = [
      "Fabio N. Filasieno"
    ];
  };
}