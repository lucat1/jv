<p align="center"> <code>jv</code> • Simple JSON Viewer </p>

![proj](https://user-images.githubusercontent.com/29304787/86464293-79098100-bd2f-11ea-8eb0-c75a1c61abbf.png)

# Dependencies jv requires the following software to be installed:

- json-c

Please make sure to install the corresponding development packages in
case that you want to build jc on a distribution with separate
runtime and development packages (e.g. _-dev on Debian, _-devel on
Void).

# Building jc is built using the commands:

```
$ make
# make install
```

Please note, that the latter one
requires root privileges. By default, jc is installed using the
prefix "/usr/local", so the full path of the executable will be
"/usr/local/bin/jc".

You can install jc into a directory of your choice by changing the
second command to:

```
# make PREFIX="/your/dir" install
```

A `DESTDIR` option is also supported. It will be appended before the `PREFIX`.

# Usage jv accepts any input from `stdin` in the form or JSON logs or

raw strings and pretty prints them to `stdout`.
