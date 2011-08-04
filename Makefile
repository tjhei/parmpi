opt: parmpi

parmpi: Makefile parmpi.cc
	mpic++ -Wall -O2 parmpi.cc -o parmpi
