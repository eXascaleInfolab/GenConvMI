
from __future__ import print_function

from gecmim.plot import get_discrete_moves
from gecmim.generation_models import two_disjoint_sets
from gecmim.movements import leaf_gains_parent
from gecmim.inclusion_bags import get_elements_from_assorted_list

import numpy as np
import matplotlib.pyplot as plt
from cPickle import load, dump

MODULE_SIZE=600
TIC_SIZE = 50
MOVE_RANGE=550
MOVE_COUNT= int( MOVE_RANGE / TIC_SIZE )
EXPER_TIMES = 10


def main():
    # Get a couple of modules
    a = np.zeros( (MOVE_COUNT+1,2), dtype=np.float )
    a[:,0] = np.arange( 0, MOVE_COUNT+1)*TIC_SIZE
    for ti in xrange( EXPER_TIMES ):
        modules = two_disjoint_sets(MODULE_SIZE,MODULE_SIZE)
        [m1, m2] = modules 
        elements = get_elements_from_assorted_list( modules )
        # Two movements
        mov1 = leaf_gains_parent( m1, m2 )
        mov2 = leaf_gains_parent( m2, m1 )
        moves = [mov1, mov2]
        drift_data = get_discrete_moves(moves, elements, None, TIC_SIZE, MOVE_COUNT )
        dd_arr = np.array( drift_data )
        a[:,1] += dd_arr[:,1]
    #print( a / EXPER_TIMES )
    a[:,1] /= EXPER_TIMES
    ofile = open("just_colliding.pck", "wb")
    dump(a, ofile, 2 )
    ofile.close()
    fig = plt.figure()
    ax = fig.add_subplot( 111 )
    ax.plot( a[:,0], a[:,1] )
    plt.show()
    
    
main()
