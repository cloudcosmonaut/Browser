#!/usr/bin/env bash

# Remove all apps folders
find ./packaging_win/share/icons/hicolor/ -type d -name "*apps" -prune -exec rm -rf {} +


