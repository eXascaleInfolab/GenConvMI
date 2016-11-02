# 
from __future__ import print_function
import gecmi


# Use this to check the help of the function gecmi.calc_nmi_and_error
#   
#   help(gecmi.calc_nmi_and_error)


# We play with something that can be converted to a numpy array
# of integers. The array should have two columns, and one row for
# each vertex-module membership.
arr0 = [
        [0,1],  # Node 0 belongs to module 1
        [1,0],  # Node 1 belongs to the module 0
       ]


# Now let's see how to compare this cover with itself. If our approach is correct we should get the NMI value 1.0 (or close):
gecmi.calc_nmi_and_error(arr0, arr0, 0.1, 0.1 )

# More complicated "covers"

# Now we are going for something more complicated. Suppose that the nodes of the network are identified with natural numbers from 0 till 100, and that there are, say, 11 modules:

straight_cover=[]
for i in xrange(100):
    straight_cover.append( (i, i%11) )


# This cover still is pretty basic, and when compared with itself should be one:
print( gecmi.calc_nmi_and_error(straight_cover, straight_cover, 0.1, 0.1 ) )

# Let's put a bit of overlapping:

not_so_straight_cover = []
for i in xrange(100):
    x = i%11
    not_so_straight_cover.append( (i, x) )
    if i % 3 == 0:
        not_so_straight_cover.append( (i, (x-1)%11 ) )

print( gecmi.calc_nmi_and_error(straight_cover, not_so_straight_cover, 0.1, 0.1 ) )

# We got an NMI value which is not as high this time. Are the error values consistent? 
# Now we play with the parameters of error and tolerance.
print( gecmi.calc_nmi_and_error(straight_cover, not_so_straight_cover, 0.001, 0.001 ) )
print( gecmi.calc_nmi_and_error(straight_cover, not_so_straight_cover, 0.001, 0.0001 ) )

# It seems to me that they are!

