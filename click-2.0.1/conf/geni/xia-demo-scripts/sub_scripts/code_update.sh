#!/bin/bash

cd ~/xia-core
git pull
cd click-2.0
make elemlist
make
cd ../XIASocket/API
make clean
make
cd ../sample
make

