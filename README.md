# fs (alternative `ls` command)

***This repository is the C++ version of [fs](https://github.com/mass-0910/fs). From now on, we will develop and update fs in this repository.***

The substitute of powershell ls command.

- Display files in grid or list format
- Display file type
- Display vertical and horizontal size of image file
- Display full path
- Show only files with a specific extension
- Highlight files with a specific extension

## How to use

You can use this code only on windows OS and Linux (and maybe MacOS).

This project uses CMake on compiling.

Compile:
```
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

Show all files in the current directry:
```
fs
```

Show usage:
```
fs -h
```

## Make configuration file ".fsrc"

You can create a configuration file under the user directry.
```
> notepad C:/Users/<username>/.fsrc
```

In ".fsrc", you can set the number of columns to display the file and the output color.

For example:
```
FILENAME_MAX = 24
COLUMN_SIZE  = 4
USE_COLOR    = TRUE
DIR_COLOR    = GREEN
LINK_COLOR   = BLUE
```