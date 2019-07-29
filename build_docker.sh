#!/bin/bash

_THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

(cd $_THIS_DIR && docker build . -t ffmpeg-playground)
