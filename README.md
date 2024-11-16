### File system related CLI utils with defaults that I like

#### Commands
- _ls_: defaults to showing real directory size 1 level deep instead of the block size. default sorting by last modified time.
- _f_: multi threaded find. recursive by default. supports simple patterns like `?`, `*` (not `[]` classes).
- _cp_: multi threaded copy. mimics the unix `cp` in behavior.
- _new_: one command for creating a file or a dir (`-d`) in the pwd.
- _size_: total size of a directory. basically multi-threaded `du -sh`.

Doesn't support unicode (yet).

### Installation
__Create executable__
```sh
make
```

__Install with the command name `fs`__
```sh
make install
```

__Run tests__
```sh
make test
```

### Development checklist
- [x] Build a cli parser

- [x] Add and configure a test runner

```sh
fs .
```
- [x] prints out the files, directories of the current (or any) directory. defaults
to list view. has human readable sizes and permissions and date modified. default
sorted by date modified. has the actual directory size instead of the block size.
shows hidden files by default.
```sh
fs . -s -a
```
- [x] same as above but -s sorts by size and -a sorts alphabetically

```sh
fs f . --file "hello.c"
```
- [x] means find and print full paths of any file in the current directory with name
containing the string "hello.c". recursive by default. the name can also be a 
pattern like "*.mp4" etc. maybe make search multi threaded??

```sh
fs f . --file "hello.c" -nr
```
- [x] nr means no recursion so only search the top-level dir

```sh
fs cp <source> <destination>
```
- [x] copy from source to destination. if destination don't exist it creates one.
source can be file or directory

```sh
fs new "file.txt"
```
- [x] creates a file named "file.txt" in the current directory

```sh
fs new -d "my_dev"
```
- [x] creates a directory named "my_dev" in the current directory

```sh
fs size .
```
- [x] gives the total space occupied on the disk by the current directory or file