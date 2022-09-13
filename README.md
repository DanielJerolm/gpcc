# gpcc
The swiss army knife for C++ cross-platform embedded software.

# Before cloning...
GPCC is intended to be used as a git submodule in a git superproject. Cloning the gpcc repo alone does not make sense
and is of limited use in most scenarios. It may even be confusing for new users.

To evaluate or develop gpcc on a Linux host, please clone __gpcc_dev__, which contains GPCC as a git submodule and
provides a fully configured VSCODE workspace and CMake files for all build configurations:

HTTPS: git clone -b master --recurse-submodules https://github.com/DanielJerolm/gpcc_dev.git
SSH: git clone -b master --recurse-submodules git@github.com:DanielJerolm/gpcc_dev.git
