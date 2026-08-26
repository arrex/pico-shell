### [2026-08-26]
in the middle of writing the file system component for this project. this task turned
out to be a lot harder than i thought. the foundational layers of the file system are
complete along with unit tests. some info below:

1. disk layer -> we emulate the disk with an in-memory byte array.

2. block layer -> sits on top of disk. we introduce the abstraction of blocks which help
   us access the disk in a more structured way.

3. file system layout -> determines how we want to layout the file systems (how many inodes,
   how many blocks, etc.). i'm leaning towards a simple extent-based allocation policy.

4. inode layer -> sits on top of the block layer, also relies on the file system layout
   defined. 

5. data block layer -> sits on top of the block layer, also relies on the file system layout
   defined. adjacent to inode layer.

as next steps, i want to build the directory layer on top of the inode/data block abstractions
to handle directory lookup/create/delete use cases. then, we can move on to handling file 
operations as well.

this component of the system could also become some good practice for writing concurrent C.

### [2026-08-20]
this is the first devlog entry. i figured having this log file would help with
documenting the development process of this passion project :)

refactored parts of the code base:
- removed the custom `my_` prefix from commands since it was annoying to type out
- split frame store and variable store source code and make them sub-components of an 
  overarching memory manager component
- add better memory management with cleanup `deinit` functions
- other small cleanup tasks

current fully-fledged functionalities:
- shell commands
- scheduling with different policies via the `exec` command
- demand paging is fully implemented with LRU frame eviction
