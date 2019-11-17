#!/bin/sh

wget -P @PROJECT_SOURCE_DIR@/third_party https://github.com/vovkos/doxyrest/releases/download/doxyrest-2.0.0/doxyrest-2.0.0-linux-amd64.tar.xz
tar xf @PROJECT_SOURCE_DIR@/third_party/doxyrest-2.0.0-linux-amd64.tar.xz -C @PROJECT_SOURCE_DIR@/third_party/
mv @PROJECT_SOURCE_DIR@/third_party/doxyrest-2.0.0-linux-amd64 @PROJECT_SOURCE_DIR@/third_party/doxyrest
rm @PROJECT_SOURCE_DIR@/third_party/doxyrest-2.0.0-linux-amd64.tar.xz
