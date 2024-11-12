### program for some fs related things i like

### TODOS

- [x] Build a cli parser

- [x] add and configure a test runner

```sh
fsc .
```
- [x] prints out the files, directories of the current (or any) directory. defaults
to list view. has human readable sizes and permissions and date modified. default
sorted by date modified. has the actual directory size instead of the block size.
shows hidden files by default
```sh
fsc . -s -a
```
- [x] same as above but -s sorts by size and -a sorts alphabetically

```sh
fsc f . --file "hello.c"
```
- [x] means find and print full paths of any file in the current directory with name
containing the string "hello.c". recursive by default. the name can also be a 
pattern like "*.mp4" etc. maybe make search multi threaded??

```sh
fsc f . --file "hello.c" -nr
```
- [x] nr means no recursion so only search the top-level dir

```sh
fsc cp <source> <destination>
```
- [ ] copy from source to destination. if destination don't exist it creates one.
source can be file or directory

```sh
fsc mv <source> <destination>
```
- [ ] move from source to destination. if destination don't exist it creates one.
source can be file or directory

```sh
fsc new "file.txt"
```
- [x] creates a file named "file.txt" in the current directory

```sh
fsc new -d "my_dev"
```
- [x] creates a directory named "my_dev" in the current directory

```sh
fsc stat .
```
- [ ] gives the total space occupied on the disk by the current directory or file