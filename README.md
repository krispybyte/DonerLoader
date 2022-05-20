# Information
This is an asynchronous client and server project using boost's TCP library, asio.
Also written in C++20 due to the ease of development when building an asynchronous application using the new co routines.

# Task-list
- [x] Public key exchange.
- [x] Aes key and initialization vector exchange.
- [ ] Module streaming.
- [ ] Hardware ID check.
- [ ] Heartbeat.
- [ ] Database implementation using mongodb.
- [ ] Module injection.
- [ ] Module relocations and imports handled over the server.
- [ ] Client state verification handled over the server.
- [ ] Small gui?
- [ ] Client pre-loader in the future.
- [ ] SHA256 for json key values.

# Setup
### Cloning
cd to the directory you wanna clone the repository into, and enter the following command.
```
git clone --recursive https://github.com/stackey-net/Network.git
```

#### CryptoPP library compilation
1.  cd to the Dependencies/cryptopp directory.
2.  git clone https://github.com/noloader/cryptopp-pem.
3.  Add pem.h, pem_common.cpp, pem_common.h, pem_read.cpp, pem_write.cpp, x509cert.cpp and x509cert.h from the crypto-pem directory to the cryptopp submodule.
4.  Launch the project and include the files you've just added, compile in x64 architecture.

#### MongoDb CXX library compilation
1.  cd over to the Dependencies/mongo-c-driver directory.
2.  cmake -G "Visual Studio 16 2019" -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_FLAGS="/Zc:__cplusplus" -DCMAKE_PREFIX_PATH=D:\Network\Dependencies\mongo-c-driver -DCMAKE_INSTALL_PREFIX=D:\Network\Dependencies-mongo-cxx-driver
3.  Launch mongo-c-driver.sln, make sure the architecture is set to x64.
4.  Compile the needed projects now, or all of them.
5.  Launch CMake GUI, The source code path will be the directory of your mongo-cxx-driver, for example: D:\Network\Dependencies\mongo-cxx-driver.
6.  Press 'configure', set the generator and press finish.
7.  You should get an error, make sure to turn BSONCXX_POLY_USE_STD on.
8.  Make sure the 'Advanced' checkbox is turned on, scroll down to the bottom, you should see that bson-1.0_DIR and libbson-1.0_DIR are missing, set the libbson directory to the 'mongo-c-driver/src/libbson' directory, and the bson directory to the 'mongo-c-driver/src/libbson/bson', obviously include the full path for both.
9.  You should get another error, with invalid libmongoc and mongoc directories, make the libmongoc path the 'mongo-c-driver/libmongoc' directory and the mongoc path the 'mongo-c-driver/libmongoc/mongoc' directory before pressing 'Configure' again.
10.  Press 'Add Entry', name it 'CMAKE_CXX_STANDARD', set the type to string and the value to 20. Now edit the 'CMAKE_CXX_FLAGS' field and add "/Zc:__cplusplus" to it.
11.  Configuration should be finished, press 'Generate' and then 'Open Project'.
12.  Head to the 'mongo-cxx-driver/build' directory, launch MONGO_CXX_DRIVER.sln.
13.  Make sure the architecture is set to x64, now compile 'mongocxx_shared'.
14.  Done, the library will be in the 'mongo-cxx-driver/build/src/mongocxx/Release' directory by default.
15.  In your actual project, make sure to also include the bson library, and both of the dynamic libraries.

# Dependencies
- [Asio](https://github.com/chriskohlhoff/asio "Asio on github")
- [Cryptopp](https://github.com/weidai11/cryptopp "Cryptopp on github")
- [Json](https://github.com/nlohmann/json "Json on github")
- [MongoDB CXX](https://github.com/mongodb/mongo-cxx-driver "MongoDB CXX on github")
