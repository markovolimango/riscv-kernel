# riscv-kernel

it's a kernel

## Implementation details

### Memory allocation

Since memory has to be alligned to blocks and the system call 0x01 (mem_alloc) has to take the number of blocks, this is a bit weird.
We can't just add a footer, because it would take up a whole block.
And without a footer, coalescing is O(n).
So the solution is: between every two segments of allocated or free data, we have a metadata block, which essentially acts like the header of the segment after and the footer of the segment before.
This makes operations trickier, but should be the best solution for the requirements.

### Threads

#### Priority

Priorities range from 0 to 15, with 0 being the lowest and 15 being the highest.
