# coding: utf-8

from __future__ import print_function

from gecmim.validation_suite.splitting_confusion import SplittingConfusion
from gecmim.estimate import estimate
from gecmi import vertex_module_bimap_t

sc = SplittingConfusion(object_count=1024)

#import yaml

results = []
for i in range( 6 ):
    sc.advance()
    (vmb1, vmb2) = sc.get_partitions()
    assert isinstance( vmb2, vertex_module_bimap_t )
    #print( vmb2.to_tuples() )
    e = estimate( vmb1, vmb2, 0.01, 100, verbose=0)
    results.append( {'i': i, 'estimate': e } )
    print( i, e['nmi'] )
    
#with open( 'results1.yaml', 'w') as out:
    #yaml.dump( results, out )
