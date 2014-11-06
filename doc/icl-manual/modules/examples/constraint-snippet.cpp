SixDofConstraint *c = new SixDOFConstraint(objectA,objectB,
                                           Mat(1,0,0,0,
                                               0,1,0,0,
                                               0,0,1,-1,
                                               0,0,0,1),
                                           Mat(1,0,0,0,
                                               0,1,0,0,
                                               0,0,1,1,
                                               0,0,0,1));
c->setAngularLowerLimit(Vec(0,0,0));
c->setAngularUpperLimit(Vec(0,0,M_PI));
