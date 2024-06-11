#!/bin/sh

clang -cc1 -Itestdata -ast-dump $1
