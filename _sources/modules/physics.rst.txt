.. include:: ../js.rst

.. _physics:

###########################
3D Physics Simulation
###########################

.. image:: /icons/185px/physics.png

The ICLPhysics package provides a powerful wrapper layer around the
most common aspects of the versatile Bullet Physics engine
(http://bulletphysics.org). The implemented wrapper layout features
soft and rigid body dynamics and collisions as well as constraints and motor
types. Its seamless integration with ICLGeom's rendering API allows for super fast
rapid prototyping.

.. image:: images/collision.png 
 :scale: 30%

Table of Contents
^^^^^^^^^^^^^^^^^

* :ref:`physics.physicsscene`
* :ref:`physics.rigidobject`
* :ref:`physics.softobject`
* :ref:`physics.constraint`
* :ref:`physics.dimensions`


.. _physics.physicsscene:

:icl:`PhysicsScene`
^^^^^^^^^^^^^^^^^^^^^^^

The :icl:`PhysicsScene` extends the :icl:`PhysicsWorld` and the :icl:`Scene`.
It provides global settings like gravity and collision groups, as well as utility
functions to test rays as well as objects for collision with the objects within the
PhysicsWorld.
:icl:`PhysicsObject` and :icl:`Constraint` can be added and behave just
like :icl:`SceneObjects`. The simulation can be stepped through my simply calling
the step function. For further information on how to fine-tune the step 
parameters visit http://bulletphysics.org/mediawiki-1.5.8/index.php/Stepping_The_World.

.. _physics.rigidobject:

:icl:`RigidObject`
^^^^^^^^^^^^^^^^^^^^^^^

The :icl:`physics::RigidObject` class is one of the two basic types of objects in
the physics scene. The subclasses like :icl:`RigidBoxObject` or 
:icl:`RigidCylinderObject` allow for quick scene building with primitive shapes.
For more complex shapes :icl:`RigidConvexHullObject` can be used to create a
:icl:`RigidObject` from a vector of points. The :icl:`RigidCompoundObject`
allows to create concave objects by combining other :icl:`RigidObjects`.
:icl:`RigidObject` have  parameters like weight, friction, or
restitution which allows for modeling different behaviour when objects 
collide.

Here is an example for a simple scene with some objects:

.. literalinclude:: examples/physics-scene.cpp
  :language: c++
  :linenos:

.. _physics.softobject:

:icl:`SoftObject`
^^^^^^^^^^^^^^^^^^^^^^^

The :icl:`SoftObject` is the wrapper class for the bullet softbodies. Softbodies
enable simulation of pliable bodies like cloth and rubber at a greater cost of
performance. 

.. _physics.constraint:

:icl:`Constraints`
^^^^^^^^^^^^^^^^^^^^^^^

:icl:`Constraints` allow the simulation of behaviour like hinges, ball sockets 
or rails. These constraints are all implemented based on the 
:icl:`SixDofConstraint` which allows for generic constraints. The 
constraint works by describing 2 transformations matrices which represnt a positoins and orientation (in bullet these are called frames) relativ to two objects. When all six degrees of freedem(3 translation and 3 rotation degrees) are locked the constraint will try move these frames on top of each other. By unlocking
certain degrees free traversal along that degree can be allowed.

The following code will create an axis with the length of 2 units between the objects A and B, where both objects can turn at most rotate 180° away from each other:

.. literalinclude:: examples/constraint-snippet.cpp
  :language: c++
  :linenos:

For more instructions on how constraints work check out the physics-constraint and the physics-car demo or visit http://bulletphysics.org/mediawiki-1.5.8/index.php/Constraints.

.. _physics.dimensions:

:icl:`Dimensions`
^^^^^^^^^^^^^^^^^^^^^^^

When constructing scenes it is always important to keep an eye on proportions. The physics work best when objects have a size of about 1.
This is where the two global variables ICL_UNIT_TO_METER and METER_TO_BULLET_UNIT come into play. The default unit in ICL is the millimeter therefore ICL_UNIT_TO_METER is 0.001. When working with objects that are about 10 cm in size METER_TO_BULLET_UNIT should be set to 0.1. Since objects in the ICL usually are about this big 0.1 is the default value. One of the limitations of the bullet physics engine is the interaction between very small/light and big/heavy objects. In
these cases rounding errors can result in unexpected behaviour.
