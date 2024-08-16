#!/bin/bash

# Places zip in build directory.

if [ ! -e ../build/index.html ]; then
  echo "Could not find built index.html file. Run build-emscripten.sh first."
  exit 1
fi

set -eu

unset workdir
onexit() {
  if [ -n ${workdir-} ]; then
    rm -rf "$workdir"
  fi
}
trap onexit EXIT

workdir=$(mktemp --tmpdir -d gitzip.XXXXXX)

cp -a ../build/index.html ../build/index.data ../build/index.js ../build/index.wasm "$workdir"

pushd "$workdir"
git init
git config --local user.email "zip@example.com"
git config --local user.name "zip"
git add .
git commit -m "commit for zip"
popd

git archive --format=zip -o ../build/itch.zip --remote="$workdir" HEAD