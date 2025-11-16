{ lib
, stdenv
, nodejs
, esbuild
, vsce
, xibVscodeNative ? null
}:

stdenv.mkDerivation rec {
  pname = "xib-vscode-plugin";
  version = "0.0.1";

  src = ./.;

  nativeBuildInputs = [ nodejs esbuild vsce ];

  # Build: transpile TS with esbuild (no network), build native addon with node-gyp, package vsix
  buildPhase = ''
    runHook preBuild
    mkdir -p dist
    esbuild src/extension.ts --bundle --platform=node --format=cjs --external:vscode --outfile=dist/extension.js

    if [ -n "${xibVscodeNative}" ]; then
      echo "Bundling prebuilt native addon from dependency"
      mkdir -p native/build/Release
      cp -v ${xibVscodeNative}/lib/addon.node native/build/Release/addon.node
    else
      echo "WARNING: xibVscodeNative not provided; native addon will be missing."
    fi

    echo "Packaging VSIX"
    vsce package --no-dependencies --out xib-vscode-plugin.vsix
    runHook postBuild
  '';

  installPhase = ''
    runHook preInstall
    mkdir -p "$out"
    cp xib-vscode-plugin.vsix "$out"/
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


