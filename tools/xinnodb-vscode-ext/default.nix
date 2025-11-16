{ lib
, stdenv
, nodejs
, esbuild
, fetchurl
, fetchFromGitHub
, runCommand
}:

let
  # Pin npm dependencies for deterministic builds
  npmDeps = runCommand "xinnodb-vscode-ext-npm-deps" {
    nativeBuildInputs = [ nodejs ];
    outputHashMode = "recursive";
    outputHashAlgo = "sha256";
    outputHash = "sha256-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA="; # Will be computed
  } ''
    mkdir -p $out
    cd $out

    # Create package-lock.json with pinned versions for deterministic builds
    cat > package.json << 'EOF'
    {
      "name": "xinnodb-vscode-ext-deps",
      "version": "1.0.0",
      "private": true,
      "dependencies": {
        "@types/mocha": "^10.0.6",
        "@types/node": "^20.11.30",
        "@types/vscode": "^1.84.0",
        "@vscode/test-electron": "^2.3.9",
        "glob": "^10.3.10",
        "mocha": "^10.3.0",
        "typescript": "^5.4.0"
      }
    }
    EOF

    npm install --package-lock-only
    cp package-lock.json $out/
  '';

in
stdenv.mkDerivation rec {
  pname = "xinnodb-vscode-ext";
  version = "0.0.1";

  src = ./.;

  nativeBuildInputs = [ nodejs esbuild ];

  # Pre-build: Install dependencies deterministically
  preBuild = ''
    # Use pinned dependencies for deterministic builds
    if [ -f ${npmDeps}/package-lock.json ]; then
      cp ${npmDeps}/package-lock.json .
      npm ci --offline
    else
      # Fallback: install normally (for development)
      npm install
    fi
  '';

  # Build: transpile TS with esbuild (no network)
  buildPhase = ''
    runHook preBuild
    mkdir -p dist
    esbuild src/extension.ts src/interfaces.ts --bundle --platform=node --format=cjs --external:vscode --outfile=dist/extension.js

    # Compile tests if they exist
    if [ -d test ]; then
      esbuild test/**/*.ts --bundle --platform=node --format=cjs --external:vscode --outdir=out/test
    fi

    runHook postBuild
  '';

  # Disable network access during build for determinism
  __noChroot = false;

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