from __future__ import print_function

import random

from gecmim.plot import get_discrete_moves
from gecmim.generation_models import two_disjoint_sets, uniform_arbitrary
from gecmim.movements import entity_diffussion, apply_movements
from gecmim.inclusion_bags import get_elements_from_assorted_list
from gecmim.inclusion_bags import ibags_to_vertex_map_bimap, \
     ibags_to_file
from gecmim.io_utils import vmb_to_file

MODULE_COUNT = 10
ENTITY_COUNT = 500
MOVE_COUNT = 100

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
    with open("csample1.clu", "w") as csample1:
        vmb_to_file( vmb0, csample1 )

    # Apply a few moves
    apply_movements( moves, times = MOVE_COUNT )
    vmb = ibags_to_vertex_map_bimap( entities, numbering_context )
    with open("csample2.clu", "w") as csample2:
        vmb_to_file( vmb, csample2 )
        

main()

