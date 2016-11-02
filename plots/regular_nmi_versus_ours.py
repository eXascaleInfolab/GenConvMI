#!
from __future__ import print_function

import json
from matplotlib import pyplot as plt
import numpy as np 
import sys
# Data comes from another script....

def main():
    data = json.load( sys.stdin  )
    fig = plt.figure()
    ax = fig.add_subplot( 111 )
    mtx = np.array( data )
    x = np.arange( len( mtx ) )
    ax.plot( x, mtx[:,0], '.', x, mtx[:,1], 'o' )
    plt.show()

main()
