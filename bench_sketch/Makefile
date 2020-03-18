all: bench_update_gen data4hash data4hash2 bench_query_gen bench_query_gen4Real bench_query_gen4Real2 bench_client bench_client_r bench_client4Real cut

bench_update_gen: bench_update_gen.cc
	g++ -std=c++0x -o bench_update_gen bench_update_gen.cc -lssl -lcrypto -lz -lmemcached -I/user/local/include/libmemcached -L/usr/lib -L/usr/local/lib

bench_query_gen: bench_query_gen.cc
	g++ -std=c++0x -o bench_query_gen bench_query_gen.cc -lssl -lcrypto -lz

bench_query_gen4Real: bench_query_gen4Real.cc
	g++ -std=c++0x -o bench_query_gen4Real bench_query_gen4Real.cc -lssl -lcrypto -lz
bench_query_gen4Real2: bench_query_gen4Real2.cc
	g++ -std=c++0x -o bench_query_gen4Real2 bench_query_gen4Real2.cc -lssl -lcrypto -lz

data4hash: data4hash.o city.o
	g++ -std=c++0x -o data4hash city.o data4hash.o -lssl -lcrypto -lz

data4hash.o: data4hash.cc
	g++ -std=c++0x -O2 -c data4hash.cc -I/usr/include -I/usr/local/include

data4hash2: data4hash2.o city.o
	g++ -std=c++0x -o data4hash2 city.o data4hash2.o -lssl -lcrypto -lz

data4hash2.o: data4hash2.cc
	g++ -std=c++0x -O2 -c data4hash2.cc -I/usr/include -I/usr/local/include

city.o: city.c
	g++ -std=gnu99 -O2 -c city.c -I/usr/include -I/usr/local/include

bench_client: bench_client.o
	gcc -std=gnu99 -O2 -o bench_client bench_client.o -lpthread  -lmemcached -L/usr/lib -L/usr/local/lib

bench_client.o: bench_client.c
	gcc -std=gnu99 -O2 -c bench_client.c -I/usr/include -I/usr/local/include -I/user/local/include/libmemcached

bench_client_r: bench_client_r.o
	gcc -std=gnu99 -O2 -o bench_client_r bench_client_r.o  -lmemcached -L/usr/lib -L/usr/local/lib

bench_client_r.o: bench_client_r.c
	gcc -std=gnu99 -O2 -c bench_client_r.c -I/usr/include -I/usr/local/include -I/user/local/include/libmemcached


bench_client4Real: bench_client4Real.o
	gcc -std=gnu99 -O2 -o bench_client4Real bench_client4Real.o -lpthread  -lmemcached -L/usr/lib -L/usr/local/lib

bench_client4Real.o: bench_client4Real.c
	gcc -std=gnu99 -O2 -c bench_client4Real.c -I/usr/include -I/usr/local/include -I/user/local/include/libmemcached

cut: cut.o
	gcc -std=gnu99 -O2 -o cut cut.o -L/usr/lib -L/usr/local/lib

cut.o: cut.c
	gcc -std=gnu99 -O2 -c cut.c -I/usr/include -I/usr/local/include

clean:
	-rm *.o bench_client bench_client_r bench_client4Real bench_update_gen bench_query_gen bench_query_gen4Real bench_query_gen4Real2 cut
