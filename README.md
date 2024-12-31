# NETWORKING FILES SYSTEM
## Contributuions: 
* Ansh Acharya: Naming server, Storage Server, Debugging.
* Adithya Addepalli Casichetty: Naming Server, Storage Server ,Debugging.
* Karthik Venkat Malavathula: Naming Server, Client, Storage Server ,Debugging.
* Sanyam Agrawal: Client, Storage Server, Debugging.
## How to run?
* After cloning the repo, go to `/namingserver` and type `gcc main.c -o ns` then `./ns` on your shell to setup naming server.
* Then go to `/storageserver` and type `gcc main.c -o ss` then `./ss` on your shell to setup up storage srver.
* Then go to `/client` and type `make` then `./a.out` on your shell to set up client.
* After sending the file paths from SS to NS you are ready to go :smiley:.
* Note: Each of these instructions should be performed in different terminals.
## Assumptions
* The files could be of `.txt`, `.c`, `.h` and `.mp3` format only.
* If the user provides a folder it will also provides all the files inside that folder to naming server.
* Across all storage servers, any two files or folder can't have same name.
## Naming Server
* On it's initialization, NS creates two threads for registering Storage Servers and for handling incoming client requests.
* For both of the communcations we are using TCP protocol for better reliability.
* NS receives paths of files and directories from the storage server along with the two ports the storage server uses(one for communication with the NS and the other for communication with the client).
* Whenever a client makes a request, NS first searches the cache for the file, and returns the appropriate SS IP and port.
* If the file is not found in the cache, then it searches through the paths received from all the SSs and then sends IP and port number of appropriate SS if the file is found.
* Backup thingie.
 
## Storage Server
* On it's initialization, storage server adds the file paths to the naimg server along with it's IP and port number. 
## Client 
* On it's initialization, storage server adds the file paths to the naimg server along with it's IP and port number.
