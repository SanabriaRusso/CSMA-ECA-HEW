This simulator is part of a PhD thesis. It is provided for free and without any warranty so you can use it, modify it and test your protocols.


CSMA/ECA Implementation
------------------------

These source files are part of the UPF's current project that aims at simulating Ken Duffy's proposal of deterministic backoff timers and the concept of stickiness.

At the moment, CSMA/ECA is implemented with both Hysteresis and Fair Share mechanisms. Please refer to [1] to learn more about the protocol behavior and results.

[1]: http://arxiv.org/abs/1303.3734

Luis.

============
CSMA-ECA-HEW
============

Modelling of Carrier Sense Multiple Access with Enhanced Collision Avoidance for the upcoming High-Efficiency WLANs standard.

This attempt will modify:

	-Symbol times to comply with 802.11ac
	-Better queuing mechanisms: in previous attempts the saturated queue made the simulations to take too long.
	-Better channel models: in order to comply with the scenarios, new channel models need to be addressed.
	-Implementation of coverage areas and communication ranges.

Personal:
Important files for making the figures are found in ../tmp/plot/multipleAC/paper
