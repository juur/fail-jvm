fail-jvm is a partial implementation of the following:

## The Java(R) Virtual Machine specification, Java SE 7 Edition

* class File parsing
* 126 / 200+ JVM instructions ("byte code")
* threading, monitor locks

## The Java(R) Platform, Standard Edition 6, API Specification

The following packages have some implemented classes based on the published API Specification:

* java.io
* java.lang
* java.lang.annotation
* java.lang.reflect
* java.util
* java.nio
* java.nio.channels
* java.nio.channels.spi
* java.nio.charset
* java.nio.charset.spi
* java.math
* java.text

## Usage

type make to compile

by default looks in /usr/lib/jvm/java-1.6.0 for bin/javac. See Makefile top for overrides.

to run, place Java source in a 'test' folder, and run with two arguments: ClassFile.class ClassFile
Any java source in the test folder will be compiled

Oracle and Java are registered trademarks of Oracle and/or its affiliates. Other names may be trademarks of their respective owners.
