Compile
```
$ make
```


Read Write
```
$ ./pgc write 1 123
$ ./pgc read 1
```
->Result
```
reading physical memory
reading page table
reading data from virtual address:1
-> Physical address:258049, Value:123
```
