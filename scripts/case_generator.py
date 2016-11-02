
from __future__ import print_function

from gecmim.plot import get_discrete_moves
from gecmim.generation_models import two_disjoint_sets, uniform_arbitrary, \
     uniform_flat
from gecmim.movements import entity_diffussion, apply_movements
from gecmim.inclusion_bags import get_elements_from_assorted_list
from gecmim.inclusion_bags import ibags_to_vertex_map_bimap, \
     ibags_to_file, ibags_to_std_file
from gecmim.estimate import estimate

import numpy as np
import random
import os.path

def gen_flat_case_0():
    MOVE_COUNT = 100
    MODULE_COUNT = 30
    ENTITY_COUNT = 2000
    out_filename_0 = os.path.join( 'data', 'flat_case_0_0.txt' )
    out_filename_1 = os.path.join( 'data', 'flat_case_0_1.txt' )
    
    random.seed(3439)
    numbering_context = dict()
    
    (modules, entities) = uniform_flat( MODULE_COUNT, ENTITY_COUNT, 1 )
    with open( out_filename_0, 'w' ) as out:
        ibags_to_std_file( entities, numbering_context, out )

    (modules, entities) = uniform_flat( modules, entities, 1 )
    with open( out_filename_1, 'w' ) as out:
        ibags_to_std_file( entities, numbering_context, out )
    
def gen_flat_case_1():
    MOVE_COUNT = 100
    MODULE_COUNT = 30
    ENTITY_COUNT = 2000
    out_filename_0 = os.path.join( 'data', 'flat_case_1_0.txt' )
    out_filename_1 = os.path.join( 'data', 'flat_case_1_1.txt' )
    
    random.seed(3439)
    numbering_context = dict()
    
    (modules, entities) = uniform_flat( MODULE_COUNT, ENTITY_COUNT, 2 )
    with open( out_filename_0, 'w' ) as out:
        ibags_to_std_file( entities, numbering_context, out )

    (modules, entities) = uniform_flat( modules, entities, 2 )
    with open( out_filename_1, 'w' ) as out:
        ibags_to_std_file( entities, numbering_context, out )        

def main():
    gen_flat_case_0()
    gen_flat_case_1()
    
main()
