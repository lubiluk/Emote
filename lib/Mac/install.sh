#!/bin/bash

# Installs D2XX library to system

sudo cp libftd2xx.1.1.0.dylib /usr/local/lib/libftd2xx.1.1.0.dylib
sudo ln -sf /usr/local/lib/libftd2xx.1.1.0.dylib /usr/local/lib/libftd2xx.dylib

sudo cp ftd2xx.h /usr/local/include/ftd2xx.h
sudo cp WinTypes.h /usr/local/include/WinTypes.h