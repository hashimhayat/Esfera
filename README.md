### <i class="icon-file"></ Esfera - A Realtime 360 Video Delivery Protocol
----------

Creating an Executable & Testing
A simple Makefile is created to generate an executable of the Implementation. The executable is called ***lab4***.

In order to test the protocol implementation perform the following steps:

 1. Go to the lab4 directory.
 2. Use the *make* command to create an exe.
 3. To run the exe, use: *./esfera -n N -p "P" -c C*
    - N: number of threads
    - P: refers to the path of the folder where you want to store the files. Create a new folder outside the project directory.
    - C: size of cache
     ~~~~
    - example: *./esfera -n 4 -p "/home/something/storage" -c 128*
     ~~~~
 4. Note that the default port for the ThreadPoolServer is **8001**.



