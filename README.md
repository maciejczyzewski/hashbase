<div align="center">
  <img src="https://raw.githubusercontent.com/MaciejCzyzewski/hashbase/master/screenshot-1.png"/>
</div>

# hashbase [![Build Status](https://travis-ci.org/MaciejCzyzewski/hashbase.png)](https://travis-ci.org/MaciejCzyzewski/hashbase)

Flexible hackable-storage.

## Introduction

It's a fast, efficient on-disk/in-memory database with many different kind of data structures.

#### More reading:

- [Installation](#installation): Step-by-step instructions for getting hashbase running on your computer.
- [Contributing](#contributing): Explanation of how you can join the project.
- [License](#license): Clarification of certain rules.

## Installation

When an archive file of hashbase is extracted, change the current working directory to the generated directory and perform installation.

Run the configuration script.

```bash
$ autoreconf -fvi
$ ./configure
```

Build programs.

```bash
$ make
```

Perform self-diagnostic test. This takes a while.

```bash
$ make check
```

Install programs. This operation must be carried out by the root user.

```bash
$ make install
```

## Contributing

Please feel free to contribute to this project! Pull requests and feature requests welcome! :v:

[![Gitter chat](https://badges.gitter.im/MaciejCzyzewski/hashbase.png)](https://gitter.im/MaciejCzyzewski/hashbase)

## License

See LICENSE file in this repository.