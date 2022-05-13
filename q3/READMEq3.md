## README

To run the programs, first run server program(`server_prog.cpp`) and then client program(`client_sim.cpp`).

For compiling the server program, run:

`g++ server_prog.cpp -lpthread -o server`

To run the server program:

`./server`

For compiling the client program, run:

`g++ client_sim.cpp -lpthread -o client`

To run the server program:

`./client`

Enter the input as specified in the assignment pdf.
Sample Input(on client side):

```
11
1 insert 1 hello
2 insert 1 hello
2 insert 2 yes
2 insert 3 no
3 concat 1 2
3 concat 1 3
4 delete 3
5 delete 4
6 concat 1 4
7 update 1 final
8 concat 1 2
```



Sample Output(on the client side):

```
0:140537819449088:Insertion Successful
1:140537811056384:Key Already Exists
3:140537802663680:Insertion Successful
2:140537794270976:Insertion Successful
4:140537785878272:yeshello
5:140537819449088:nohelloyes
6:140537811056384:Deletion Successful
7:140537794270976:No such key exists
8:140537802663680:Concat failed as atleast one of the keys does not exists
9:140537785878272:final
10:140537819449088:yeshellofinal
```

