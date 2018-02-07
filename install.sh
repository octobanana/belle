#!/usr/bin/env bash
set -e

# source environment variables
source ./env.sh

BUILD_TYPE="Debug"

if [[ $# > 0 ]]; then
  if [[ $1 == "-d" ]]; then
    BUILD_TYPE="Debug"
  elif [[ $1 == "-r" ]]; then
    BUILD_TYPE="Release"
  fi
fi

if [[ ${BUILD_TYPE} == "Debug" ]]; then
  printf "\nInstalling Debug ${APP}\n"
  mkdir -vp /usr/local/include/ob
  cp -vr ./include/belle.hh /usr/local/include/ob/
  cp -v ./build/debug/libbelle.so /usr/local/lib/

elif [[ ${BUILD_TYPE} == "Release" ]]; then
  printf "\nInstalling Release ${APP}\n"
  mkdir -vp /usr/local/include/ob
  cp -vr ./include/belle.hh /usr/local/include/ob/
  cp -v ./build/release/libbelle.so /usr/local/lib/
fi
