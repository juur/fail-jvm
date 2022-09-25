fail-jvm is a partial implementation of the following:

## The Java(R) Virtual Machine specification, Java SE 6 (and bits of 7) Edition

* .class file parsing, linking, etc. 
* JVM execution
* 196 / 201 JVM instructions ("byte code") supported [From Java SE 6]
* threading, monitor locks

Some features of the classfile are acknowledged, not supported:

* BootstrapMethods

Some features of the classfile are parsed, but no-op:

* StackMapTable

Unimplemented instructions:

* dup\_x2
* dup2\_x1
* dup2\_x2
* jsr
* jsr\_w

## The Java(R) Platform, Standard Edition 6, API Specification

The following packages have some implemented classes based on the published API Specification:

* java.io
* java.lang
* java.lang.annotation
* java.lang.reflect
* java.math
* java.net
* java.nio
* java.nio.channels
* java.nio.channels.spi
* java.nio.charset
* java.nio.charset.spi
* java.text
* java.util
* java.util.logging
* java.util.regex
* java.util.zip

## Usage

type make to compile. does not support parallel make (no dependencies for java classes in cp/)

by default looks in /usr/lib/jvm/java-1.8.0 for bin/javac. See Makefile top for overrides.

to run, place Java source in a 'test' folder, and run with two arguments: path/to/ClassFile.class ClassFile
Any java source in a folder called "test" will also be compiled

Oracle and Java are registered trademarks of Oracle and/or its affiliates. Other names may be trademarks of their respective owners.
