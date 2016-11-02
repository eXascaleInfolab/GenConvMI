"""

Here we examine how the nmi estimation varies as the number of datapoints increases.

"""

from __future__ import print_function

from gecmi import nmi_exact_driver_routine
from gecmi.event_generator import deep_complete_simulator, \
     nmi_from_event_strip, counter_matrix_t

from gecmim.plot import get_discrete_moves
from gecmim.generation_models import two_disjoint_sets, uniform_arbitrary
from gecmim.movements import entity_diffussion, apply_movements
from gecmim.inclusion_bags import get_elements_from_assorted_list
from gecmim.inclusion_bags import ibags_to_vertex_map_bimap, \
     ibags_to_file



import numpy as np
import random
import matplotlib.pyplot as plt
from cPickle import load, dump

MOVE_COUNT= 20

SIMULATION_GRANULARITY = 10
SIMULATION_IN_GRANULARITY_STEPS = 10000

MODULE_COUNT = 5
ENTITY_COUNT = 60

# What's going on here? We check how, when getting more and more samples,
# we get closer or not to the m.i. 
def main():
    random.seed(3439)
    (modules, entities) = uniform_arbitrary( MODULE_COUNT, ENTITY_COUNT )
    mov = entity_diffussion( entities, modules )
    # Two movements
    moves = [mov,]
    numbering_context = dict()
    
    vmb0 = ibags_to_vertex_map_bimap( entities , numbering_context )
    
    # Apply a few moves
    apply_movements( moves, times = MOVE_COUNT )
    vmb = ibags_to_vertex_map_bimap( entities, numbering_context )

    #nmi = nmi_exact_driver_routine( vmb0, vmb )
    #print("Exact value: ", nmi )
    
    # Now let's see how we can converge to an approximate value
    dcs = deep_complete_simulator(vmb0, vmb )
    counter_mat = counter_matrix_t( MODULE_COUNT, MODULE_COUNT )
    ofile = open("variance_game.txt", "w" )
    for i in xrange( SIMULATION_IN_GRANULARITY_STEPS ):
        evs = dcs.get_n_events( SIMULATION_GRANULARITY )
        counter_mat.assimilate_events( evs )
        print( counter_mat.calculate_nmi(), file=ofile )
    ofile.close()
    #print( evs.shape )
    #print( nmi_from_event_strip( evs, MODULE_COUNT, MODULE_COUNT ) )
    
main()
