# Unique Words Counter

## Introduction
Program for counting unique words in file delivered as filepath for program argument.
Words are splitted by newline and space char `' '`.
Program will operate on maximum number of threads available by the system.

For large text files, processing might take a while and program will not output anything to inform that its working. Only number with total unique words count will be printed at very end of the processing.

## Supported Platforms

* Linux

## Build

### Generate (make)build system with cmake and build

```
  $ mkdir build && cd build
  $ cmake ..
  $ make -j
```

### Run

```
  $ ./uniqueWordsCounter ../text_file_example.txt
```

Program will output a number of unique words, eg.:
```
  $ ./uniqueWordsCounter ../text_file_example.txt
  $ 64
```