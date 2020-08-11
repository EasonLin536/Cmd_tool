mypkg.d: ../../include/mypkg.h 
../../include/mypkg.h: mypkg.h
	@rm -f ../../include/mypkg.h
	@ln -fs ../src/mypkg/mypkg.h ../../include/mypkg.h
