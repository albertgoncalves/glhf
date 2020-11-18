with import <nixpkgs> {};
mkShell {
    buildInputs = [
        clang_10
        cppcheck
        glfw3
        libGL
        python3
        shellcheck
        valgrind
        xorg.libX11
        xorg.libXfixes
        xorg.libXrandr
    ];
    shellHook = ''
        export PATH_GLFW="${glfw3}"
        export PATH_LIBGL="${libGL}"
        . .shellhook
    '';
}
