CC = g++
DEBUG = -g
OBJ = main.cpp ThreadSafeKVStore/TSKVStore.cpp ThreadSafeListenerQueue/TSListenerQueue.cpp ThreadPool/ThreadPool.cpp HTTP/httpReq.cpp HTTP/httpResp.cpp MD5_HASH/md5wrapper.cpp MD5_HASH/md5.cpp Delegate/Delegate.cpp Performance/Statistics.cpp
CFLAGS  = -std=c++11 -Wall
TARGET = lab4

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o  $(TARGET)  $(OBJ) -lpthread

clean:
	$(RM) $(TARGET)
