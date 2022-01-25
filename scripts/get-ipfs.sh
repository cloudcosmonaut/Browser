#!/bin/bash
# By: Melroy van den Berg
# Description: Automatically download and unzip the IPFS CLI binaries for Linux & Windows

## Provide the IPFS version below ##
IPFS_VERSION=0.11.0

##############################
# Leave the code alone below #
##############################
CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

OS=`uname`

echo "INFO: Start downloading IPFS (version ${IPFS_VERSION})..."
if [ "$OS" == "Linux" ]; then
    echo "Downloading IPFS binaries for Linux"
    wget --quiet "https://dist.ipfs.io/go-ipfs/v${IPFS_VERSION}/go-ipfs_v${IPFS_VERSION}_linux-amd64.tar.gz" -O "$CURRENT_DIR/go-ipfs_linux.tar.gz"
    echo "Extracting IPFS binaries for Linux"
    tar -xzf "$CURRENT_DIR/go-ipfs_linux.tar.gz" -C "$CURRENT_DIR/../"
    rm "$CURRENT_DIR/go-ipfs_linux.tar.gz"
elif [ "$OS" == "Darwin" ]; then
    echo "Downloading IPFS binaries for MacOS"
    wget --quiet "https://dist.ipfs.io/go-ipfs/v${IPFS_VERSION}/go-ipfs_v${IPFS_VERSION}_darwin-amd64.tar.gz" -O "$CURRENT_DIR/go-ipfs_darwin-amd64.tar.gz"
    echo "Extracting IPFS binaries for MacOS"
    tar -xzf "$CURRENT_DIR/go-ipfs_darwin-amd64.tar.gz" -C "$CURRENT_DIR/../"
    rm "$CURRENT_DIR/go-ipfs_darwin-amd64.tar.gz"
else
    echo "Downloading IPFS binaries for Windows"
    wget --quiet "https://dist.ipfs.io/go-ipfs/v${IPFS_VERSION}/go-ipfs_v${IPFS_VERSION}_windows-amd64.zip" -O "$CURRENT_DIR/go-ipfs_windows.zip"
    echo "Extracting IPFS binaries for Windows"
    unzip go-ipfs_windows.zip -d "$CURRENT_DIR/../"
    rm "$CURRENT_DIR/go-ipfs_windows.zip"
fi
