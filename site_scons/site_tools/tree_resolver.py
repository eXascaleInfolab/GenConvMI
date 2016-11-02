"""

Given two trees as yaml files (sources) and a file 
of fallback rules (resolver, also yaml), creates two 
new versions of the sources with common leaf nodes.

"""

from __future__ import print_function



def TOOL_TREE_RESOLVER( env ):
    """A Tool to add a header from $HEADER to the source file"""
    add_header = Builder(action=['echo "$HEADER" > $TARGET',
                                 'cat $SOURCE >> $TARGET'])
    env.Append(BUILDERS = {'AddHeader' : add_header})
    env['HEADER'] = '' # set default value