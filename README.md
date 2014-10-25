<div align="center">
  <img src="https://raw.githubusercontent.com/MaciejCzyzewski/hashbase/master/screenshot-1.png"/>
</div>

# hashbase [![Build Status](https://travis-ci.org/MaciejCzyzewski/hashbase.png)](https://travis-ci.org/MaciejCzyzewski/hashbase)

Flexible hackable-storage.

## Introduction

It's a fast, efficient on-disk/in-memory database with many different kind of data structures.

Hashbase is written in ANSI C comes with a broad range of features including metrics, multi-client API and rich advanced data structure (KV, List, Hash, ZSet, Bit).

#### More reading:

- [Installation](#installation): Step-by-step instructions for getting hashbase running on your computer.
- [Usage](#usage): List of commands.
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

## Usage

<div align="center">
  <img src="https://raw.githubusercontent.com/MaciejCzyzewski/hashbase/master/screenshot-2.png"/>
</div>

This script is the primary interface for starting and stopping the hashbase server.

### Start

To start a daemonized (background) instance of hashbase.

```bash
$ hashbase -d
```

### Console

Alternatively, if you want to run a foreground instance of hashbase.

```bash
$ hashbase
```

### Stop

Stopping a foreground or background instance of hashbase can be done from a shell prompt.

```bash
$ hashbase -s
```

### Help

Displays a brief summary of the basic options.

```bash
$ hashbase -h
```

## Contributing

Please feel free to contribute to this project! Pull requests and feature requests welcome! :v:

## License

See LICENSE file in this repository.