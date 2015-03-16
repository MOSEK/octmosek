# Welcome to the OctMOSEK project!

**This project is currently not under active developement..**

## Purpose
The Octave-to-MOSEK Optimization Interface is designed to make the optimization facilities of [MOSEK](http://www.mosek.com/) available from [Octave](http://www.gnu.org/software/octave/). MOSEK is a software library able to solve large-scale optimization problems, and supports:

  * Linear programming (LP)
  * Quadratic and All-Quadratic programming (QP and QCQP)
  * Second-Order Cone Programming (SOCP)
  * And all the above with Mixed-Integer variables..

Note that the MOSEK libraries ships with [trial and academic licenses](http://www.mosek.com/resources/trial/) free of charge, but is otherwise a proprietary product. 

## System Requirements
**You need Octave!**
Obviously, since this is an interface for [Octave](http://www.gnu.org/software/octave/). However, since stable 64 bit versions does not yet exist for all platforms you might be limited to 32 bit Octave linking to 32 bit MOSEK.

**You need MOSEK!**
Again obviously, as this is an interface to [MOSEK](http://www.mosek.com/). However, only MOSEK version 6 is supported. Free [trial and academic licenses](http://www.mosek.com/resources/trial/) are available, and [quotations](http://www.mosek.com/sales/quotation/) can be requested to allow commercial usage.

**You need the OctMOSEK source code!**
We do not distribute binary versions of the OctMOSEK package. Instead, you are referred to the **.tar.gz* file under the "Downloads" tab on this website.

## Installation
The OctMOSEK package features auto-configuration which will work in most cases if the system has been properly set up (see System Requirements). Inserting the correct location and version below, most users will be able to install the package with:
```
pkg install -verbose 'LOCATION/octmosek_VERSION.tar.gz'
```

If this does not work, we advice you to look in the [userguide](http://octmosek.googlecode.com/svn/trunk/doc/userguide.pdf) for more information.

## Getting started
The package can be loaded with
```
pkg load octmosek
```

The provided functions can be seen with
```
pkg describe -verbose octmosek
```

The manual for each function (here `mosek`) can be found with
```
help mosek
```

The `mosek` function is used to solve all supported optimization problems.

#### Example 1
In this example a linear program is defined and solved.
```
clear -v lo1;
lo1.sense = "max";
lo1.c = [3 1 5 1];
lo1.A = sparse([3 1 2 0;
                2 1 3 1;
                0 2 0 3]);
lo1.blc = [30 15 -Inf];
lo1.buc = [30 inf 25];
lo1.blx = [0 0 0 0];
lo1.bux = [Inf 10 Inf Inf];
r = mosek(lo1);
```

#### Example 2
In this example the formulation of a mixed integer linear program is read from a file and solved. The first line finds the root of the OctMOSEK package, and the second extracts the path of the *milo1.opf* file within. Alternatives are *lo1.opf* and *cqo1.opf*.
```
pkg_root = pkg("list"){strcmp({[pkg("list"){:}].name},"octmosek")}.dir;
modelfile = fullfile(pkg_root, "extdata", "milo1.opf");
r_read = mosek_read(modelfile);
r_solve = mosek(r_read.prob);
```

## Tips and tricks, questions, feature requests and bug reports
We invite all users of the Octave-to-MOSEK Interface to join the [MOSEK Google group](http://groups.google.com/group/mosek). This group is particularly intended for academics using MOSEK in their research. 
