### <i class="icon-file"></i> Lab 4 - ThreadPoolServer with File Storage and Cache in C++
----------

Creating an Executable & Testing
A simple Makefile is created to generate an executable of the Implementation. The executable is called ***lab4***.

In order to test the lab implementation perform the following steps:

 1. Go to the lab4 directory.
 2. Use the *make* command to create an exe.
 3. To run the exe, use: *./lab4 -n N -p "P" -c C*
    - N: number of threads
    - P: refers to the path of the folder where you want to store the files. Create a new folder outside the project directory.
    - C: size of cache
     ~~~~
    - example: *./lab4 -n 4 -p "/home/something/storage" -c 128*
     ~~~~
 4. Note that the default port for the ThreadPoolServer is **8001**.
 5. Use a Request generator such as httperf to connect to the ThreadPoolServer and give it tasks:
 6. Testing: Httperf usage: <br>
 ~~~~
 httperf --server localhost --port 8001 --wsesslog=2,1,/home/hh1316/MulticoreProgramming/lab4/TEST/zipfian_test.txt
 ~~~~
 7. To view logs (mean, median, min, max, etc) use: *curl localhost:8001/LOGS* or use a web browser to send a get request to LOGS.
 8. Use make clean to remove executable and debug files.

 ***Performance***

 In this lab, we implemented a file management system to store, retrieve and delete files on the disks. We also implemented a LRU Cache to act as an intermediary between the thread pool server and the file system. The performance was measured with and without using a cache. The results were as expected. With a cache a much better performance was seen. Moreover, a larger cache would produce better performance.




