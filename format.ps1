Get-ChildItem -Path src -Directory -Recurse |
    foreach {
        pushd $_.FullName
        &clang-format -i  *.cc
        &clang-format -i  *.h
        popd
    }