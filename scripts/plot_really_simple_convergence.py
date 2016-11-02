from __future__ import print_function

import numpy as np
import matplotlib.pyplot as plt

def main():
    # Really cool function: genfromtxt. 
    # The file "variance_game.txt" is generated executing 
    # 'variance_collection.py'
    data = np.genfromtxt( "variance_game.txt" )
    fig = plt.figure()
    ax = fig.add_subplot( 211)
    ax.plot( data,  )
    ax.set_ylim( [0,1] )
    ax = fig.add_subplot( 212)
    ax.plot( data,  )
    #ax.set_ylim( [0.8,0.9] )
    plt.show()
    
if __name__ == '__main__':
    main()
