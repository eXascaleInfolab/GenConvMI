# GenConvMI

Generalized Conventional Mutual Information (NMI for Overlapping clusters compatible with standard NMI)
Gecmi evaluates the mutual information of graph covers considering overlaps.  
The paper: [Comparing network covers using mutual information](https://arxiv.org/abs/1202.0425) by Alcides Viamontes Esquivel, Martin Rosval, 2012.  
(c) Alcides Viamontes Esquivel

This is a clone of the slightly outdated [gecmi repository on bitbucket](https://bitbucket.org/dsign/gecmi) with the fixed compilation under Linux Ubuntu 16.04 x64 and minor I/O extensions to support stabdard formats and be easily applicable in the [PyCaBeM](https://github.com/eXascaleInfolab/PyCABeM) clustering benchmark.  
Modified and extended by Artem Lutov <artem@exascale.info>

*The refined, optimized and extended, pure C++ version with the fully automatic cross-platform build producing a single executable is available in the [GenConvNMI](https://github.com/eXascaleInfolab/GenConvNMI) repository.*

## Content

- [Deployment](#deployment)
	- [Dependencies](#dependencies)
	- [Compilation](#compilation)
	- [Installation](#installation)
- [Usage](#usage)
	- [Executable](#executable)
	- [Python Module](#python-module)
- [Related Projects](#related-projects)

# Deployment

The prebuilt binaries for *Ubuntu 16.04 x64* can be downloaded in the [releases section](https://github.com/eXascaleInfolab/GenConvMI/releases), the dependencies
should be additionally installed as outlined below.

## Dependencies

For the *compilation*:
- [boost](http://www.boost.org/boost) >= v.1.47
- [itbb](http://threadingbuildingblocks.org/itbb) >= v.3.0
- [scons](http://www.scons.org/scons) >= v.2.0
You will additionally need g++ >= v.4.6
> Any lower version will also probably work after some tuning.

For the *brebuilt executables*:
- libtbb2:  `$ sudo apt-get install libtbb2`
- libboost_program_options v1.58:  `$ sudo apt-get install libboost-program-options1.58.0`
- libstdc++6:  `$ sudo apt-get install libstdc++6`

For using the Python module, you will need development headers of python,
boost::python (including in boost, which is required anyway) and *numpy*.

* [scipy](http://numpy.scipy.org/)
* [python](http://www.python.org/)

## Compilation

Once you download and unpack the source code of *Gecmi* in a directory,
`cd` inside it and look for a file called `site_config.py.example`, and
rename or copy this file so that you get `site_config.py` in the same
directory. Check this file and edit it in such a way that it matches your build
environment, the targets you want to compile and where do you want to install
them.

Finally, do
```
$ scons
```

and you will have the build going.

Things to watch out:

* You can get messages of the kind `error while loading shared libraries` if the dependencies
  are not correctly installed. In that case, you might want to fiddle with the commands
  `locate` and the environment variable `LD_LIBRARY_PATH`, or the equivalents in your
  operating system of choice.

* I have only tested the build setup in Linux.

## Installation

When the build finish, you can install components using

```
$ sudo scons install
```
Alternatively you can copy-paste two libraries located from `build/objects-release/lib` and
- `build/bin/gecmi` to run the (C++) executable
- `build/python/gecmi.so` to import from the Python
to the required directory. Note that `libcluster_reader.so` and `libgecmi_so.so` should be
located in the same directory as `gecmi`, or in the 'lib/' subdirectory, otherwise you will
have to define `LD_LIBRARY_PATH` to start `gecmi`.

If everything worked o.k, you should be able to run `gecmi` executable, or import it from
Python as `import gecmi` using `gecmi.so` module.

# Usage

## Executable

The standalone program uses files in CNL format:

```
# The comments start with '#' like this line
# Each non-commented line is a module(cluster, community) consisting of the the member nodes separated by space / tab
1
1 2
2
```
where each line corresponds to the network nodes forming the cluster (community, module).
> To run the executable, it's local dependencies should be located in the same directory or int the 'lib/' subdirectory.

The original `gecmi` format is also supported:

```
vertex: modules
0: 1
1: 1 2
2: 2
```
means a network cover with the vertices 0,1,2. The vertex numbers appear before
the colon. The vertex numbers should be consecutive and start from zero, but they do not need  
to appear in the file in this way. The modules of
each vertex appear after the colon, separated by spaces, and there is a line for each
vertex and its memberships. If you prefer, it is
also possible to have the file in the opposite format:
```
module: vertices
1: 0 1
2: 2
```

*The format is automatically identified by the file header (or it's absence).*

To get the normalized mutual information of the covers in the two files, issue the
command:

```
$ gecmi file1 file2
```

If you want to tweak the precision, use the options -e and -r, to set the error and
the risk respectively. See the [paper](http://arxiv.org/abs/1202.0425) for the
meaning of these concepts.

## Python Module

The python module allows to use this tool in programs more easily. Check an [example](https://github.com/eXascaleInfolab/GenConvMI/blob/master/simple_example.ipynb)
of how it is used.

# Related Projects
- [GenConvNMI](https://github.com/eXascaleInfolab/GenConvNMI) - The refined, optimized and extended, pure C++ version of GenConvMI with the fully automatic cross-platform build that produces a single executable.
- [PyCABeM](https://github.com/eXascaleInfolab/PyCABeM) - Python Benchmarking Framework for the Clustering Algorithms Evaluation. Uses extrinsic (NMIs) and intrinsic (Q) measures for the clusters quality evaluation considering overlaps (nodes membership by multiple clusters).
