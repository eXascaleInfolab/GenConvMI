# GenConvMI

Generalized Conventional Mutual Information (NMI for Overlapping clusters compatible with standard NMI)
Gecmi evaluates the mutual information of graph covers considering overlaps.  
Paper: [Comparing network covers using mutual information](https://arxiv.org/abs/1202.0425) by Alcides Viamontes Esquivel, Martin Rosval, 2012.  
(c) Alcides Viamontes Esquivel

This is a clone of the slightly outdated [gecmi repository on bitbucket](https://bitbucket.org/dsign/gecmi) with the fixed compilation under Linux Ubuntu X64 and minor I/O extensions to support stabdard formats and be easily applicable in the [PyCaBeM](https://github.com/eXascaleInfolab/PyCABeM) clustering benchmark.  
Modified and extended by Artem Lutov <artem@exascale.info>

## Content
- [Deployment](#deployment)
	- [Dependencies](#dependencies)
	- [Compilation](#compilation)
- [Usage](#usage)
	- [Standalone Program](#standalone-program)

# Deployment
## Dependencies

* [boost](http://www.boost.org/boost) >= v.1.47
* [itbb](http://threadingbuildingblocks.org/itbb) >= v.3.0
* [scons](http://www.scons.org/scons) >= v.2.0
You will additionally need g++ >= v.4.6

> Any lower version will also probably work after some tuning.
 
For using the Python module, you will need developement headers of python,
boost::python (including in boost, which is required anyway) and numpy.

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

and you will have the build going. When the build finish, you can install components using 

```
$ sudo scons install
```

If everything worked o.k, you should be able to run a tool called `gecmi`, if you built 
the standalone program, or from python, do `import gecmi`, if you built the python module. 

Things to watch out:

* You can get messages of the kind `error while loading shared libraries` if the dependencies
  are not correctly installed. In that case, you might want to fiddle with the commands 
  `locate` and the environment variable `LD_LIBRARY_PATH`, or the equivalents in your
  operating system of choice.

* I have only tested the build setup in Linux. 

# Usage

## Standalone program

The standalone program uses files in a simple format. For example:

```
# The commentes starts with '#' like this line
# Each non-commented line is a module(cluster, community) consisting of the the member nodes separated by space / tab
1 
1 2
2
```
where each line corresponds to the network nodes forming the cluster (community, module).

The original gecmi format is also supported:

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

Note that the first line is compulsory: it indicates what is before and after the colon.

To get the normalized mutual information of the covers in the two files, issue the 
command:

```
$ gecmi file1 file2
```

If you want to tweak the precision, use the options -e and -r, to set the error and 
the risk respectively. See the [paper](http://arxiv.org/abs/1202.0425) for the 
meaning of these concepts.

## Using the python module

The python module allows to use this tool in programs more easily. Check an [example](https://github.com/eXascaleInfolab/GenConvMI/blob/master/simple_example.ipynb) 
of how it is used.
