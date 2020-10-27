# IDE
Compile instructions

#### Windows
\* Needs MinGW

##### Step 1. Clone
It's important that you don't change your PWD when cloning the three repos

```
> git clone https://github.com/MrDoritos/console
> git clone https://github.com/MrDoritos/console-ui
> git clone https://github.com/MrDoritos/ide
```

##### Step 2. Build

```
> mkdir build
> cd build
> cmake .. -G "MinGW Makefiles"
> make
```

#### Linux
Follow the same instructions as Windows, but omit `-G "MinGW Makefiles"`