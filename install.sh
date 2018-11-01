#!/usr/bin/env bash
set -e

printf "Installing Belle\n"
sudo mkdir -vp /usr/local/include/ob
sudo install -vm 644 ./include/belle.hh /usr/local/include/ob/
printf "Success!\n"
