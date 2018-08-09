# Generalized Lindenmayer System (GLS)

GLS is a language for writing L-systems with a focus on supporting very complex and realistic biological simulations amongst other applications. This is done through a series of features built upon the basic L-system structure for much more sophisticated applications contained entirely within the L-system. GLS is still in progress, but below are listed the features already implemented or planned in the near future:
* L-system symbols have a set of parameters which can be used in a production and to set conditions on the simulation. These are initially set in the axiom
* TODO: Conditions, including probabilities, using parameters can be used to have branching productions
* Hooks into C++ functions written in the format {n} where n is the index of the function pointer in C++ can be used to make far more complicated conditions and varying parameters
* TODO: GLS can set dependencies amongst symbols of different levels so that when a modification is made to one symbols parameters, all dependent symbols are notified. For example, in a biological simulation, if the branch of a tree dies or increases in some factor, all branches stemming from it should be notified and change accordingly. I have never seen this feature mentioned relating to L-Systems.
* TODO: Function overloading ?? 
