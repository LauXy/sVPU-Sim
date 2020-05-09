
#tell the linker the rpath so that we don't have to muck with LD_LIBRARY_PATH, etc
svpusim: *.cpp
	$(CXX) -g -o svpusim  --std=c++11 *.cpp -I./ -L./DRAMSim/ -ldramsim -Wl,-rpath=./DRAMSim/

clean: 
	rm svpusim