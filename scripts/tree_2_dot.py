"""

Formats a print print-out for dot

"""

from __future__ import print_function 

import sys

def memaddress2id( s, runid ):
    if s == '0':
        return 'r' + str( runid )
    s = s.replace('0x', 'n'+str(runid) )
    return s 

def main():
    # Just read it from standard input
    print( "digraph {" )
    print("graph [ rankdir = \"LR\" ];")
    node2info = {}
    state = 'watch'
    runid = 0
    for line in sys.stdin:
        if state == 'watch' and line.find( '-- begin-print_tree --') >= 0:
            state = 'processing'
            print("subgraph {" )
        elif state == 'processing':
            if line.find('-- end-print_tree --') >= 0:
                state = 'watch'
                print("}") # for subgraph
                runid += 1
            else:
                parts = line.split(' ')
                node_id_child = memaddress2id( parts[0], runid )
                node_id_parent = memaddress2id( parts[2], runid )
                if node_id_child not in node2info:
                    label = " ".join( parts[3:-1] )
                    print( "{0} [ label=\"{1}\" ];".format(node_id_child, label) )
                    node2info[ node_id_child ] = None # ??
                print( "{0} -> {1};".format( node_id_parent, node_id_child ) )
    print("}")
        
    
main()
