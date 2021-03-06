# Files to compile
SET ( FILES
  face
  facehelper
  facetpairing2
  facetpairing4
  facetspec
  pygeneric
  )

# Prepend folder name
FOREACH ( SOURCE_FILE ${FILES} )
  SET ( SOURCES ${SOURCES} generic/${SOURCE_FILE})
ENDFOREACH(SOURCE_FILE)

SET ( FILES
  boundarycomponent5
  boundarycomponent6
  boundarycomponent7
  boundarycomponent8
  boundarycomponent9
  boundarycomponent10
  boundarycomponent11
  boundarycomponent12
  boundarycomponent13
  boundarycomponent14
  boundarycomponent15
  component5
  component6
  component7
  component8
  component9
  component10
  component11
  component12
  component13
  component14
  component15
  example5
  example6
  example7
  example8
  example9
  example10
  example11
  example12
  example13
  example14
  example15
  face5
  face6
  face7
  face8
  face9
  face10
  face11
  face12
  face13
  face14
  face15
  facetpairing5
  facetpairing6
  facetpairing7
  facetpairing8
  facetpairing9
  facetpairing10
  facetpairing11
  facetpairing12
  facetpairing13
  facetpairing14
  facetpairing15
  isomorphism5
  isomorphism6
  isomorphism7
  isomorphism8
  isomorphism9
  isomorphism10
  isomorphism11
  isomorphism12
  isomorphism13
  isomorphism14
  isomorphism15
  simplex5
  simplex6
  simplex7
  simplex8
  simplex9
  simplex10
  simplex11
  simplex12
  simplex13
  simplex14
  simplex15
  triangulation5
  triangulation6
  triangulation7
  triangulation8
  triangulation9
  triangulation10
  triangulation11
  triangulation12
  triangulation13
  triangulation14
  triangulation15
  )

IF(NOT LOWDIMONLY)
  # Prepend folder name
  FOREACH ( SOURCE_FILE ${FILES} )
    SET ( SOURCES ${SOURCES} generic/${SOURCE_FILE})
  ENDFOREACH(SOURCE_FILE)
ENDIF(NOT LOWDIMONLY)

# Set the variable in the parent directory
SET( SOURCES ${SOURCES} PARENT_SCOPE)
