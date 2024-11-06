all:
	gcc -DNDEBUG -O3 -g ms1.cpp -o ms1
	gcc -DNDEBUG -O3 -g ms3.cpp -o ms3
	gcc -DNDEBUG -O3 -g -mavx -mavx2 ms4.cpp -o ms4
	gcc -DNDEBUG -O3 -g -mavx2 -mavx512f ms5.cpp -o ms5
	gcc -O0 -g ms1.cpp -o ms1.dbg
	gcc -O0 -g ms3.cpp -o ms3.dbg
	gcc -O0 -g -mavx -mavx2 ms4.cpp -o ms4.dbg
	gcc -O0 -g -mavx2 -mavx512f ms5.cpp -o ms5.dbg

clean:
	rm -f ms1 ms3 ms4 ms5 ms1.dbg ms3.dbg ms4.dbg ms5.dbg

committed:
	git status | grep "nothing to commit" 1>/dev/null

run: 
	@ echo "ms1"
	@ echo "==============================="
	@ taskset -c 2 ./ms1
	@ taskset -c 2 ./ms1
	@ taskset -c 2 ./ms1
	@ taskset -c 2 ./ms1
	@ taskset -c 2 ./ms1
	@ taskset -c 2 ./ms1
	@ taskset -c 2 ./ms1
	@ echo "ms3"
	@ echo "==============================="
	@ taskset -c 2 ./ms3
	@ taskset -c 2 ./ms3
	@ taskset -c 2 ./ms3
	@ taskset -c 2 ./ms3
	@ taskset -c 2 ./ms3
	@ taskset -c 2 ./ms3
	@ taskset -c 2 ./ms3
	@ echo "ms4"
	@ echo "==============================="
	@ taskset -c 2 ./ms4
	@ taskset -c 2 ./ms4
	@ taskset -c 2 ./ms4
	@ taskset -c 2 ./ms4
	@ taskset -c 2 ./ms4
	@ taskset -c 2 ./ms4
	@ taskset -c 2 ./ms4
	@ echo "ms5"
	@ echo "==============================="
	@ taskset -c 2 ./ms5
	@ taskset -c 2 ./ms5
	@ taskset -c 2 ./ms5
	@ taskset -c 2 ./ms5
	@ taskset -c 2 ./ms5
	@ taskset -c 2 ./ms5
	@ taskset -c 2 ./ms5

runlog: committed
	@ date >> log
	@ echo "git last commit id"
	@ git log | head -n 1 >> log
	@ echo "===============================" >> log
	@ echo "ms1" >> log
	@ echo "===============================" >> log
	@ taskset -c 2 ./ms1 >> log
	@ taskset -c 2 ./ms1 >> log
	@ taskset -c 2 ./ms1 >> log
	@ taskset -c 2 ./ms1 >> log
	@ taskset -c 2 ./ms1 >> log
	@ taskset -c 2 ./ms1 >> log
	@ taskset -c 2 ./ms1 >> log
	@ echo "ms3" >> log
	@ echo "===============================" >> log
	@ taskset -c 2 ./ms3 >> log
	@ taskset -c 2 ./ms3 >> log
	@ taskset -c 2 ./ms3 >> log
	@ taskset -c 2 ./ms3 >> log
	@ taskset -c 2 ./ms3 >> log
	@ taskset -c 2 ./ms3 >> log
	@ taskset -c 2 ./ms3 >> log
	@ echo "ms4" >> log
	@ echo "===============================" >> log
	@ taskset -c 2 ./ms4 >> log
	@ taskset -c 2 ./ms4 >> log
	@ taskset -c 2 ./ms4 >> log
	@ taskset -c 2 ./ms4 >> log
	@ taskset -c 2 ./ms4 >> log
	@ taskset -c 2 ./ms4 >> log
	@ taskset -c 2 ./ms4 >> log
	@ echo "ms5" >> log
	@ echo "===============================" >> log
	@ taskset -c 2 ./ms5 >> log
	@ taskset -c 2 ./ms5 >> log
	@ taskset -c 2 ./ms5 >> log
	@ taskset -c 2 ./ms5 >> log
	@ taskset -c 2 ./ms5 >> log
	@ taskset -c 2 ./ms5 >> log
	@ taskset -c 2 ./ms5 >> log
	@ echo >> log
	@ echo >> log
