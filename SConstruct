
# {{{ preliminaries
from __future__ import print_function
from __future__ import absolute_import

import os.path
import os
import sys

from site_config import *
# }}}

# configuring {{{

BOOST_PATH=ARGUMENTS.get('BOOST_PATH', BOOST_ROOT)

COMPILE_MODE=ARGUMENTS.get('COMPILE_MODE', COMPILE_MODE)
COMPILE_TAG=ARGUMENTS.get('COMPILE_TAG', '') # Used for distinguishing 
COMPILE_TARGETS=( 
    ARGUMENTS.get('COMPILE_TARGETS', COMPILE_TARGETS)
    ).split(' ')
print("Compiling targets", COMPILE_TARGETS )

COMPILER_NAME=ARGUMENTS.get('COMPILER_NAME', COMPILER_NAME )

BOOST_INCLUDES_PATH=os.path.join( BOOST_PATH, 'include' )
BOOST_LIBS_PATH=os.path.join( BOOST_PATH, 'lib' )

TBB_INCLUDES_PATH='-I' + os.path.join( TBB_PREFIX, 'include' )
TBB_LIBS_PATH=os.path.join( TBB_PREFIX, 'lib' )

SUBPROJECT_DIRS=[
    'alve/cluster_reader',
    'alve/gecmi',]

# }}}

# honing {{{
cpp_flags=[]
cxx_flags=['-std=c++0x', '-pthread' ]
link_flags=[]

def make_include_flags( paths, prefix='-I' ):
    if isinstance( paths, str ):
        if paths.startswith( prefix):
            return paths
        else:
            return prefix + paths 
    else:
        result = [ make_include_flags( pth, prefix ) for pth in paths ]
        return result

mif = make_include_flags

cpp_flags += [mif(BOOST_INCLUDES_PATH), mif(TBB_INCLUDES_PATH)]
compiler =  COMPILER_NAME
compile_key = COMPILE_MODE + ('_' + COMPILE_TAG if COMPILE_TAG != "" else '')
link_flags += mif( [BOOST_LIBS_PATH, TBB_LIBS_PATH], '-L' )
if COMPILE_MODE == 'debug':
    cxx_flags += [ '-g' ]
elif COMPILE_MODE == 'release' :
    cpp_flags += [ '-DNDEBUG' ]
    cxx_flags += [ '-O3' ]
else:
    print( "Unknown compile mode", file=sys.stderr )

build_path = os.path.join('build', 'objects-' + compile_key )
tlibpath = '#' + os.path.join( build_path , 'lib' )

env = Environment(
    ENV=os.environ,
    CPPPATH=[
        '#/src',
    ],
    LIBPATH=[
        tlibpath,
    ],
    CXXFLAGS= cxx_flags,
    CPPFLAGS= cpp_flags,
    LINKFLAGS= link_flags,
    CXX= compiler,
    TLIBPATH = tlibpath,
    PYTHON_PREFIX = PYTHON_PREFIX,
    PYTHON_VERSION = PYTHON_VERSION,
    COMPILE_TARGETS = COMPILE_TARGETS,
    NUMPY_PREFIX = NUMPY_PREFIX,
    PYTHON_MODULE_INSTALL_AT = PYTHON_MODULE_INSTALL_AT,
    GECMI_LIB_INSTALL_AT = GECMI_LIB_INSTALL_AT,
    GECMI_PROGRAM_INSTALL_AT = GECMI_PROGRAM_INSTALL_AT
)

env.VariantDir(
    build_path,
    'src',
    duplicate = 0
)

a_install_python_module = env.Alias("InstallPythonModule", PYTHON_MODULE_INSTALL_AT )
b_install_library = env.Alias("InstallLibrary", GECMI_LIB_INSTALL_AT )
c_install_program = env.Alias("InstallProgram", GECMI_PROGRAM_INSTALL_AT )
d_ldconfig = env.Command( 
    os.path.join(PYTHON_MODULE_INSTALL_AT, 'ld-config-ok'),
    [],
    "ldconfig" )
#if env['PLATFORM'] == 'posix':
    #env.AlwaysBuild( d_ldconfig )
    #env.Depends( d_ldconfig, b_install_library )
    #env.Depends( c_install_program, d_ldconfig )
    #env.Depends( a_install_python_module, d_ldconfig )

install_list = [ b_install_library ]
# I have the suspicion that this is not really needed.
if 'standalone' in COMPILE_TARGETS:
    install_list.append( c_install_program )
if 'python' in COMPILE_TARGETS:
    install_list.append( a_install_python_module )
d_install_all = env.Alias("install", install_list )

env.SConscript(
    dirs = map( 
        (lambda x: os.path.join( build_path, x ) ),
        SUBPROJECT_DIRS        
    ),

    exports = ['env', 'mif']
)

