import numpy as np
import matplotlib.pyplot as plt

from cPickle import load

fig = plt.figure()
ax = fig.add_subplot( 111 )
a0 = load( open("just_drift.pck") )
a1 = load( open("just_colliding.pck") )
ax.plot( a0[:,0], a0[:,1], ".",  a1[:,0], a1[:,1], "-",)
plt.show()