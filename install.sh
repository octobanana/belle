#!/usr/bin/env bash
set -e

printf "Installing\n"
sudo mkdir -vp /usr/local/include/ob
sudo cp -vr ./include/belle.hh /usr/local/include/ob/
printf "Success\n"
