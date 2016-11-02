from __future__ import print_function

"""

We use these modules (o1, o2, and s## are all modules)

           
          o1             o2
        /    \         /    \
      s11    s12     s21    s22
       
where the sets s are identical and have size `BASIC_SET_SIZE' 
and evaporate the branch s11, moving its members randomly to 
all other branches.
    
"""


from gecmim.plot import get_discrete_moves
from gecmim.generation_models import two_times_two
from gecmim.movements import leaf_drift
from gecmim.inclusion_bags import get_elements_from_assorted_list

import numpy as np
import multiprocessing as mp 

from cPickle import load, dump

BASIC_SET_SIZE = 30

MOVE_RANGE= BASIC_SET_SIZE - 1
TIC_SIZE=1
MOVE_COUNT= int( MOVE_RANGE / TIC_SIZE )
EXPER_TIMES = 10

DATA_FILENAME = "hierarchy_evaporate.pck"

def job( i ):
    modules = two_times_two(BASIC_SET_SIZE)
    elements = get_elements_from_assorted_list( modules.values() )
    
    # Movements to everywhere
    moves = []
    s11 = modules['s11']
    for name in ['s12', 's21', 's22']:
        mov = leaf_drift( s11, modules[name] )
        moves.append( mov )
    
    drift_data = get_discrete_moves(moves, elements, None, TIC_SIZE, MOVE_COUNT )
    dd_arr = np.array( drift_data )
    return dd_arr

def main():
   
    # Get a couple of modules
    a = np.zeros( (MOVE_COUNT+1,2), dtype=np.float )
    a[:,0] = np.arange( 0, MOVE_COUNT+1)*TIC_SIZE

    pool = mp.Pool( 2 )
    dd_arrs = pool.map( job, range( EXPER_TIMES ) )
    
    for dd_arr in dd_arrs:
        a[:,1] += dd_arr[:,1]

    #print( a / EXPER_TIMES )
    a[:,1] /= EXPER_TIMES
    ofile = open( DATA_FILENAME, "wb")
    dump( a, ofile, 2 )
    ofile.close()

    
def plot():
    import matplotlib.pyplot as plt
    
    fig = plt.figure()
    ax = fig.add_subplot( 111 )
    ifile = open( DATA_FILENAME, "rb" )
    a = load( ifile )
    ax.plot( a[:,0], a[:,1] )
    plt.show()
    
    
main()
