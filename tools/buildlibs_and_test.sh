#! /bin/bash

# Enclose in brackets to return to original working directory after commands have run
(

# Change to project top-level directory if script is run from within 'tools'
current_dir=${PWD##*/}
current_dir=${current_dir:-/}
if [ $current_dir = 'tools' ]; then
    cd ..
fi

# Build project
cmake -S . -B build
cmake --build build

# Run tests
cd build
ctest --verbose --output-on-failure

)

exit;
