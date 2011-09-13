################################
#
#  Sample Python Script
#
#  Illustrates progress reporting during long operations.
#
#  See the file "progress.session" for the results of running this script.
#
################################

import time

# Create an 18-tetrahedron triangulation of a knot complement with real
# boundary faces (not an ideal vertex).  The knot is L106003 from the
# knot/link census.  We used Regina to truncate the ideal vertex, and
# then copied the isomorphism signature so that we can reconstruct the
# triangulation here.
sig = 'sfLfvQvwwMQQQccjghjkmqlonrnrqpqrnsnksaisnrobocksks'
tri = regina.NTriangulation.fromIsoSig(sig)
print tri.getNumberOfTetrahedra(), 'tetrahedra'

# Create a progress manager to use during the normal surface enumeration.
# This will report the state of progress while the enumeration runs in
# the background.
manager = regina.NProgressManager()

# Start the normal surface enumeration.
# Because we are passing a progress manager to enumerate(), the
# enumeration will start in the background and control will return
# immediately to the python console.
surfaces = regina.NNormalSurfaceList.enumerate(tri,
    regina.NNormalSurfaceList.STANDARD, 1, manager)

# Wait for the surface enumeration to fully start up.
while not manager.isStarted():
    time.sleep(1)


# At this point the enumeration is up and running.
# Output a progress report every second until it finishes.
prog = manager.getProgress()
while not manager.isFinished():
    print 'Progress:', prog.getDescription()
    time.sleep(1)


# The surface enumeration is now complete.
print surfaces.getNumberOfSurfaces(), 'normal surfaces'

# Output the total time spent during the surface enumeration.
print 'Total real time:', prog.getRealTime(), 'seconds'
print 'Total cpu time:', prog.totalCPUTime(), 'seconds'
