#!/usr/bin/env bash
set -e

printf "Generating New Certificate\n"
openssl req -new -x509 -sha256 -out cert.pem -keyout key.pem -days 365 -subj /CN=127.0.0.1 -passout pass:test
