with import <nixpkgs> {};
mkShell {
    buildInputs = [
        clang_10
        cppcheck
        glfw3
        libGL
        linuxPackages.perf
        python3
        shellcheck
        valgrind
        xorg.libX11
        xorg.libXfixes
        xorg.libXrandr
    ];
    shellHook = ''
        . .shellhook
    '';
}
