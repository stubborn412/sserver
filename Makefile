sserver:ThreadPool.cpp sserver.cpp
	g++ -g -o sserver ThreadPool.cpp sserver.cpp -lpthread
sclient:ThreadPool.cpp sclient.cpp
	g++ -g -o sclient ThreadPool.cpp sclient.cpp -lpthread
test_threadpool:ThreadPool.cpp test_threadpool.cpp
	g++ -g -o test_threadpool ThreadPool.cpp test_threadpool.cpp -lpthread
