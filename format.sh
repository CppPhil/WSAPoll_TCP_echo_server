#!/bin/bash

format() {
  directory="$1"
  find "$directory" -type f \( -name '*.cpp' -or -name '*.hpp' \) -exec clang-format -i {} +
}

format "src"
format "include"

exit 0

