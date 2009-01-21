SHELL = /bin/sh

EXEC = ../bin/xyz2shp
DIFF = diff -b
CONVERT = ../bin/simple.tcl

all: test0 test1 test2

test0: $(EXEC)
	@$(EXEC) ./xyzFiles/sample.xyz foo.shp
	@$(DIFF) foo.shp ./ans0
	@$(DIFF) foo.shx ./ans0
	@$(DIFF) foo.dbf ./ans0

test1: $(EXEC) $(CONVERT)
	@$(CONVERT) ./noLatLon/maxT024.20041101.txt ./data/station.tbl maxt.xyz
	@$(EXEC) -f maxt.xyz maxT024_20041101.shp
	@$(DIFF) maxT024_20041101.shp ./ans1
	@$(DIFF) maxT024_20041101.shx ./ans1
	@$(DIFF) maxT024_20041101.dbf ./ans1

test2: $(EXEC)
	@$(EXEC) -f ./xyzFiles/sample.xyz foo2.shp
	@$(DIFF) foo2.shp ./ans2
	@$(DIFF) foo2.shx ./ans2
	@$(DIFF) foo2.dbf ./ans2

clean:
	rm -f foo.*
	rm -f maxt.xyz
	rm -f maxT024_20041101.*
	rm -f foo2.*
	rm -f memwatch.log
