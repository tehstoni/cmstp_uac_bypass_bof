#!/bin/bash
x86_64-w64-mingw32-gcc -o cmstp.x64.o -c cmstp.c -Os -DBOF -w -masm=intel -mno-stack-arg-probe
