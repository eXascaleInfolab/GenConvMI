"""

Here we examine how the nmi estimation varies as the number of datapoints increases.

"""

from __future__ import print_function

from gecmi import nmi_exact_driver_routine
from gecmi.event_generator import deep_complete_simulator, \
     counter_matrix_t

from gecmim.plot import get_discrete_moves
from gecmim.generation_models import two_disjoint_sets, uniform_arbitrary, \
     uniform_flat
from gecmim.movements import entity_diffussion, apply_movements
from gecmim.inclusion_bags import get_elements_from_assorted_list
from gecmim.inclusion_bags import ibags_to_vertex_map_bimap, \
     ibags_to_file
from gecmim.estimate import estimate

import numpy as np
import random
import matplotlib.pyplot as plt
from cPickle import load, dump

MOVE_COUNT = 100
MODULE_COUNT = 30
ENTITY_COUNT = 1000

# What's going on here? We check how, when getting more and more samples,
# we get closer or not to the m.i. 
def _test_diffusion():
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

    e_nmi = estimate( vmb0, vmb, 0.0001, 1000 )
    assert ( e_nmi['nmi'] > 0.7 )
    
def test_zero():
    random.seed(3439)
    (modules, entities) = uniform_flat( MODULE_COUNT, ENTITY_COUNT, 1 )
    numbering_context = dict()
    vmb0 = ibags_to_vertex_map_bimap( entities , numbering_context )
    (modules, entities) = uniform_flat( MODULE_COUNT, entities, 1 )
    vmb1 = ibags_to_vertex_map_bimap( entities, numbering_context )
    
    e_nmi = estimate( vmb0, vmb1, 0.0001, 1000 )
    assert ( e_nmi['nmi'] < 0.1 )
    
test_zero()
    

