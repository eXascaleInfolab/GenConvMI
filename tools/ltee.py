#!/usr/bin/env python

"""
    Replaces relative file references with absolute ones
"""

import sys
import re
import os
import os.path

_relative_path=re.compile( r'(?:^| )(src/(?:[A-Za-z0-9_-]+/)*[A-Za-z0-9_-]+\.(?:hpp|cpp|h|c))' )

def main():
    write_to_file = sys.argv[1]
    currentdir =  os.getcwd() 
    project_dir = os.environ['PROJECT_DIR']
    with open(write_to_file, 'w') as extra_out:
        line = sys.stdin.readline()
        while line != "":
            if line.find( "In file included" ) >=0:
                line = sys.stdin.readline()
                continue 
            mo = _relative_path.search( line )
            if mo:
                startpos = mo.start(1)
                endpos = mo.end(1)
                absolute_path = os.path.join(project_dir, mo.group(1) )
                relative_path = os.path.relpath( absolute_path, currentdir )
                #print >> logfile, absolute_path, relative_path, currentdir
                new_line = line[:startpos] + relative_path + line[endpos:]
            else:
                new_line = line
            sys.stdout.write( new_line )
            extra_out.write( new_line )
            sys.stdout.flush()
            line = sys.stdin.readline()


main()

