
Database relation analyzer
==========================

This is a command line program used to analyze database relations.
Given the attributes and functional relations in the relation the
program can find the keys, superkeys, if the relation is 3NF or BCNF.

To compile the program type make in a comand line.

The input files for the tool have the following format

> (The attributes in the program)
>
> (Functional 1)
>
> (Functional 2)
>
> ....
>
> (Functional n)

To see examples of input files, check out the tests folder.
Type

> dban -h

to get help for how to use the program.
