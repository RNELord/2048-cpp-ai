#!/usr/bin/env bash

cd $(dirname $0)

test -d node-server/node_modules
if [ $? != 0 ]; then
    cd node-server
    npm install --production
    cd ../
fi;

export DYLD_FALLBACK_LIBRARY_PATH="bin/"
echo "Starting websocket server..."
node ./node-server/server.js
