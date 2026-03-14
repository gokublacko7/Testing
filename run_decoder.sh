#!/bin/bash
export LD_LIBRARY_PATH=/usr/local/zxing-new/lib:$LD_LIBRARY_PATH
./build/barcode_decoder "$@"
