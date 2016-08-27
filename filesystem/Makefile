CXX = gcc
CXXFLAGS = -w -g
HEADERS = -Iio ./io/File.c


default: compile

compile:
	$(CXX) $(CXXFLAGS) ./apps/test01.c $(HEADERS) -o ./apps/test01
	$(CXX) $(CXXFLAGS) ./apps/test02.c $(HEADERS) -o ./apps/test02
	$(CXX) $(CXXFLAGS) ./apps/test03.c $(HEADERS) -o ./apps/test03
	$(CXX) $(CXXFLAGS) ./apps/test04.c $(HEADERS) -o ./apps/test04

check:
	dd if=/dev/urandom of=tesfile count=5
	./apps/test01 ./disk/vdisk
	./apps/test02 ./disk/vdisk
	./apps/test03 ./disk/vdisk
	./apps/test04 ./disk/vdisk


test01:
	./apps/test01 ./disk/vdisk
	hexdump ./disk/vdisk

test02:
	./apps/test02 ./disk/vdisk
	hexdump ./disk/vdisk

test03:
	rm outtest
	dd if=/dev/urandom of=testfile count=5
	./apps/test03 ./disk/vdisk
	hexdump ./disk/vdisk
	diff -s testfile outtest


test04:
	rm outtest
	dd if=/dev/urandom of=testfile count=5
	dd if=/dev/urandom of=testfile2 count=5
	./apps/test04 ./disk/vdisk
	hexdump ./disk/vdisk
	diff -s testfile outtest