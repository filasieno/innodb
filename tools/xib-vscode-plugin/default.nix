{ lib
, stdenv
, nodejs
, esbuild
, xibVscodeNative ? null
}:

stdenv.mkDerivation rec {
  pname = "xib-vscode-plugin";
  version = "0.0.1";

  src = ./.;

  nativeBuildInputs = [ nodejs esbuild ];

  # Build: transpile TS with esbuild (no network), bundle native addon
  buildPhase = ''
    runHook preBuild
    mkdir -p dist
    esbuild src/extension.ts --bundle --platform=node --format=cjs --external:vscode --outfile=dist/extension.js

    if [ -n "${xibVscodeNative}" ]; then
      echo "Bundling prebuilt native addon from dependency"
      mkdir -p native/build/Release
      cp -v ${xibVscodeNative}/lib/ib_node_plugin.node native/build/Release/addon.node
    else
      echo "WARNING: xibVscodeNative not provided; native addon will be missing."
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
    description = "XIB VS Code extension (Hello World) for the XInnoDB project";
    homepage = "https://github.com/sunbains/embedded-innodb";
    license = licenses.mit;
    platforms = platforms.linux;
    maintainers = [
      "Fabio N. Filasieno"
    ];
  };
}


