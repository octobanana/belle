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
  printf "\nBuilding ${APP}\n"

  printf "\nCompiling ${APP}\n"
  mkdir -p build/debug
  cd build/debug
  cmake ../../ -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
  time make
  cd ../../

elif [[ ${BUILD_TYPE} == "Release" ]]; then
  printf "\nBuilding ${APP}\n"

  printf "\nCompiling ${APP}\n"
  mkdir -p build/release
  cd build/release
  cmake ../../ -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
  time make
  cd ../../
fi
