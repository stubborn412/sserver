sserver:ThreadPool.cpp test_threadpool.cpp
	g++ -o sserver ThreadPool.cpp test_threadpool.cpp -lpthread
test_threadpool:ThreadPool.cpp test_threadpool.cpp
	g++ -o test_threadpool ThreadPool.cpp test_threadpool.cpp -lpthread
