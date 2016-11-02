"""

Every node is assigned several categories at random. 

"""
from __future__ import print_function
import numpy as np
import gecmi
import json
import sys
import math

TEST_NODE_COUNT=100
PRESET_CLASSES_COUNT=2
MAX_CLASSES_AT_NODE=3
ALPHABET_SIZE=5
MIN_CLASSES_AT_NODE=1

def iterable2class_tuple( iterable ):
     l = set( iterable ) # Ensure duplicate removal
     l = list( l )
     l.sort()
     l = tuple( l )
     return l

def generate_categories( cat_count, min_classes, max_classes, alphabet_size ):
     result = set()
     i = 0
     while len( result ) < cat_count:
          symbol_count = np.random.randint( min_classes, max_classes+1 )
          classes = np.random.randint( 0, alphabet_size, symbol_count )
          classes_tp = iterable2class_tuple( classes )
          result.add( classes_tp )
     return result

class multi2unique(object):
     def __init__(self, category_list ):
          self._cat2id = {}
          for (i,cat_set) in enumerate( category_list ):
               self._cat2id[ cat_set ] = i 
          self._id = len( category_list )
          
     def add( self, new_cat ):
          if new_cat in self._cat2id:
               return self._cat2id[ new_cat ]
          else:
               new_id = self._id 
               self._id += 1 
               self._cat2id[ new_cat ] = new_id 
               return new_id 
          
def mutate_cat( cat, alphabet_size ):
     pos = np.random.randint(0, len( cat ) )
     fail = True 
     while fail:
          new_sym = np.random.randint( 0, alphabet_size)
          fail = cat[ pos ] == new_sym

     new_cat = list( cat )
     new_cat[pos] = new_sym 
     return iterable2class_tuple( new_cat )

def generate_population():
     created_categories = generate_categories( PRESET_CLASSES_COUNT, 
                                              MIN_CLASSES_AT_NODE, MAX_CLASSES_AT_NODE,
                                              ALPHABET_SIZE )
     created_categories_as_list = list(created_categories )
     #preset_categories = np.array(created_categories_as_list)
     preset_categories = created_categories_as_list # It is ok with a synonym...
     m2u = multi2unique( preset_categories )
     n2c_mixed = []
     n2c_unique = []
     result_mixed = []
     result_unique = []
     for node_serial in xrange( TEST_NODE_COUNT ):
          idx = np.random.randint( 0, PRESET_CLASSES_COUNT )
          classes_at_node = preset_categories[idx]
          result_unique.append( (node_serial, idx ) )
          for class_ in classes_at_node:
               result_mixed.append( (node_serial, class_ ) )
          n2c_mixed.append( classes_at_node )
          n2c_unique.append( idx )
     return (result_mixed, result_unique, n2c_mixed, n2c_unique, m2u)

def perturbate_populations( 
          m2u,
          n2c_unique,  # <-- List/array
          n2c_mixed,   # <-- List/array
          perturbation_steps_count 
     ):
     """
     This function returns a new tuple with:
     
     (result_mixed, result_unique, n2c_mixed, n2c_unique, m2u)
     
     Note that the returned m2u instance is the same one passed 
     as input, as the updates to this structure should be non-destructive.
     
     """
     assert isinstance( m2u, multi2unique )
     assert  isinstance( n2c_mixed, list )
     assert isinstance( n2c_unique, list )
     node_count = len( n2c_unique )
     n2c_mixed = list( n2c_mixed )
     n2c_unique = list( n2c_unique )
     result_mixed = []
     result_unique = []
     for i in xrange( perturbation_steps_count ):
          # Pickup a node 
          which_node = np.random.randint( 0, node_count )
          # Alter it slightly 
          mixed_cat = n2c_mixed[ which_node ]
          idx = n2c_unique[ which_node ]
          new_cat = mutate_cat( mixed_cat, ALPHABET_SIZE )
          assert( new_cat != mixed_cat )

          new_idx = m2u.add( new_cat )
          assert( new_idx != idx ) 
          
          n2c_unique[ which_node ] = new_idx 
          n2c_mixed[ which_node ] = new_cat
          
     for which_node in xrange( node_count ):
          new_cat  = n2c_mixed[ which_node ]
          for cl in new_cat:
               result_mixed.append( (which_node,cl) )
          new_idx = n2c_unique[ which_node ]
          result_unique.append( (which_node, new_idx) )
          
     return (result_mixed, result_unique, n2c_mixed, n2c_unique, m2u)
          
def compare_populations_mixed( res_mix_0, res_mix_1 ):

     nmi = gecmi.calc_nmi_and_error( res_mix_0, res_mix_1, 0.05, 0.02 )

     return nmi 

def compare_populations_unique( res_uni_0, res_uni_1):
     # Not very different indeed!
     nmi = gecmi.calc_nmi_and_error( res_uni_0, res_uni_1, 0.05, 0.02 )
     return nmi
          
def sanity_1():
     """ Just see if everything is working as expected """
     population_0 = generate_population()
     #print( population_0 )
     (result_mixed0, result_unique0, n2c_mixed0, n2c_unique0, m2u0) = population_0
     nmi = compare_populations_mixed( result_mixed0, result_mixed0 )
     print( nmi )
     population_1 = perturbate_populations( 
          m2u0, n2c_unique0, n2c_mixed0, 400 )
     (result_mixed1, result_unique1, n2c_mixed1, n2c_unique1, m2u1) = population_1
     c = 0
     for i in xrange( TEST_NODE_COUNT ):
          idx0 = n2c_unique0[ i ]
          idx1 = n2c_unique1[ i ]
          if idx0 != idx1 :
               c += 1
     print( "Differences:", c )
     nmi = compare_populations_mixed( result_mixed0, result_mixed1 )
     print("Our nmi", nmi )
     nmi2 = compare_populations_unique( result_unique0, result_unique1 )
     print("Conventional nmi", nmi2 )

def measure_1():
     """ Collect data required for the first perturbation plot"""
     population_0 = generate_population()
     #print( population_0 )
     (result_mixed0, result_unique0, n2c_mixed0, n2c_unique0, m2u0) = population_0

     INNER_LOOP_COUNT = 10
     plot_data = []
     for i in xrange( 0, TEST_NODE_COUNT ):
          snmi0 = 0.0
          snmi1 = 0.0
          for j in xrange( INNER_LOOP_COUNT ):
               population_1 = perturbate_populations( 
                    m2u0, n2c_unique0, n2c_mixed0, i )
               (result_mixed1, result_unique1, n2c_mixed1, n2c_unique1, m2u1) = population_1
               try:
                    nmi0 = compare_populations_mixed( result_mixed0, result_mixed1 )
               except RuntimeError:
                    print( "Mixed 0" )
                    print( n2c_mixed0 )
                    print( "Mixed 1" )
                    print( n2c_mixed1 )
                    raise
                           
               nmi1 = compare_populations_unique( result_unique0, result_unique1 )
               
               snmi0 += nmi0['nmi']
               snmi1 += nmi1['nmi']
          nmi0 = snmi0 / INNER_LOOP_COUNT
          nmi1 = snmi1 / INNER_LOOP_COUNT
          print(i, file=sys.stderr )

          plot_data.append( ( nmi0, nmi1 ) )
     print( json.dumps(plot_data, indent=4) )
     #print("Done")

if __name__ == '__main__':
     measure_1()
     #sanity_1()
