from __future__ import print_function

from gecmi import nmi_exact_driver_routine
from gecmi.event_generator import deep_complete_simulator, \
     nmi_from_event_strip

from gecmim.plot import get_discrete_moves
from gecmim.generation_models import two_disjoint_sets
from gecmim.movements import leaf_drift, apply_movements
from gecmim.inclusion_bags import get_elements_from_assorted_list
from gecmim.inclusion_bags import ibags_to_vertex_map_bimap, \
     ibags_to_file



import numpy as np
import random
import matplotlib.pyplot as plt
from cPickle import load, dump

MODULE_SIZE=600
MOVE_COUNT= 90
EXPER_TIMES = 10

# What's going on here? We check how, when getting more and more samples,
# we get closer or not to the m.i. 
def main():
    modules = two_disjoint_sets(MODULE_SIZE,MODULE_SIZE)
    [m1, m2] = modules 
    elements = get_elements_from_assorted_list( modules )
    # Two movements
    random.seed(3439)
    mov1 = leaf_drift( m1, m2 )
    mov2 = leaf_drift( m2, m1 )
    moves = [mov1, mov2]
    numbering_context = dict()
    
    vmb0 = ibags_to_vertex_map_bimap( elements , numbering_context )
    
    # Apply a few moves
    apply_movements( moves, times = MOVE_COUNT )
    vmb = ibags_to_vertex_map_bimap( elements, numbering_context )

    nmi = nmi_exact_driver_routine( vmb0, vmb )
    print("Exact value: ", nmi )
    
    # Now let's see how we can converge to an approximate value
    dcs = deep_complete_simulator(vmb0, vmb )
    evs = dcs.get_n_events( 240000 )
    print( evs.shape )
    print( nmi_from_event_strip( evs, 2, 2 ) )
    
main()
