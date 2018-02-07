#!/usr/bin/env bash
set -e

# source environment variables
source ./env.sh

printf "\nGenerating Docs for ${APP}\n"
doxygen .doxyfile
