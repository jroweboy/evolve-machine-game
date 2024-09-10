Huffmunch Output Data Format
============================

Huffmunch is a compression format that implements huffman coding,
but with output symbols that can be multiple bytes, and can share suffixes.
This allows better compression than huffman alone, as the enlarged symbols
and shared suffixes provide a dictionary mechanism similar to that used
in LZ compression methods, but with extremely small memory requirements.


The compressed output bank has three parts:

1. Header - describes the location and size of the stored bitstreams
2. Tree - describes how to decode the bitstreams
3. Bitstreams - the data to be decoded

A multibank output has an extra bank table in a separate file:

4. Bank Table - table for determining the bank each bitstream resides in



There are 3 data types that appear in this document:

HEAD = unsigned integer (16-bit by default)
BYTE = unsigned 8-bit integer
WORD = unsigned 16-bit integer
INTX = BYTE (if 0-254), or BYTE 255 followed by WORD
STREAM = bitstream



Header
======

1. HEAD - how many streams of data are contained
2. HEAD x count - beginning of each data stream, relative to start of header
3. HEAD x count - size of each data stream

The header is designed so that it could easily be omitted from the output,
if the user wants to encode the positions in a different way. For example,
in many situations the length of uncompressed data may already be known.

To accomplish that, any offsets in the tree section of the data are relative to the tree,
rather than the start of the output binary, so the entire header can safely be stripped
if not needed.

Also, the start of the first bitstream can be found immediately following the
tree structure. In standard form, this can be found taking right nodes until
finding a leaf, then advancing to the end of that leaf's data.



Tree
====

The head node of the tree appears immediately following the header.
There are several types of node, identified by their first byte.


Node 0: leaf, a single byte to emit

1. BYTE - 0
2. BYTE - byte to emit


Node 1: leaf, a string of bytes to emit

1. BYTE - 1
2. BYTE - length of string (1-255)
3. BYTE x length - bytes to emit


Node 2: leaf, a string prefix to emit followed by a link to another leaf
After emitting the byte string contained in this leaf, jump to the new leaf and emit it.

1. BYTE - 2
2. BYTE - length of prefix (1-255)
3. BYTE x length - bytes to emit
4. WORD - offset to another leaf, relative to head of tree


Node 3-254: short branch
This node branches to two other nodes, selected by the next bit in the bitstream.
The left branch is taken by a 0, and the right branch by a 1.

1. BYTE - 3-254 offset to right node

The left node begins at this node + 1.
The right node begins at this node + offset.


Node 255: long branch
Similar to the short branch, but the first byte of 255 indicates a 16-bit offset.

1. INTX - offset to right node

The left node begins at this node + 3.
The right node begins at this node + offset.


Bitstream
=========

Each stored bitstream begins on a byte boundary, specified in the header section.
Bits of the stream are stored in each byte starting with the most significant bit first.

The first bit of the stream comes from bit 7 of the first byte.
The second bit comes from bit 6...
The eighth bit comes from bit 0...
The ninth bit comes from bit 7 of the second byte...
Etc.

For each bit read, take the corresponding left or right branch of the tree until a
leaf is found, then emit the string designated by that leaf, before returning to the
head of the tree.

Note that if a tree contains only a single leaf, no bits are needed to encode it.
For this degenerate case, the same symbol should be emitted as many times as needed
to fill up the length of uncompressed data specified by the header.



Bank Table
==========

This table is generated by the huffmunch command line utility, but is not used by the
huffmunch 6502 runtime library. You must write your own code to select a bank,
then run huffmunch from that bank's data. This table can be used to assist that.


This file is a simple list of integers:

1. HEAD x number of banks - index of first entry past this bank.


Example: we have 5 banks with 50 entries. The table looks like:
[ 13 24 46 50 50 ]

If we're looking for entry 35:
- Check bank 0: 13 <= 35, go to next bank.
- Check bank 1: 24 <= 35, go to next bank.
- Check bank 2: 46 > 35, use this bank.
- Bank 2 started at 24, so subtract this from the entry: 35 - 24 = 11.
- Switch to bank 2, point huffmunch at its data and load entry 11.


As an alternative, you could ignore this bank table and simply read the first
HEAD entry from each bank's Header. This is always a count of the number of
entries in that bank. This method would look like:
- Bank 0: 13 entries: 13 <= 35, subtract and go to next bank: 35 - 13 = 22.
- Bank 1: 11 entries: 11 <= 22, subtract and go to next bank: 22 - 11 = 11.
- Bank 2: 22 entries: 22 > 11, use this bank.
- Switch to bank 2, point huffmunch at its data and load entry 11.


The advantage of using the generated bank table is that it provides the bank information
without having to read from each bank's header. This may not be very useful with
random access ROM, but if each "bank" is a data file on a floppy disk, or some other
slow medium, this bank table might be more appropriate.



List File
=========

This is the input file for multi-entry, or multi-bank data.
This text can be displayed by running the command line tool with no arguments.

List files are a simple text format:
    Line 1: (banks) (size)
        banks (int) - maximum number of banks to split output into
                      use 0 for unlimited banks
                      use 1 if multiple banks are not needed (faster)
        size (int) - how many bytes allowed in each bank
    Lines 2+: (start) (end) (file)
        start (int) - first byte to read from file
        end (int) - last byte to read from file + 1
                    use -1 to read the whole file
        file - name of file extends to end of line
    The input sources will be compressed together and packed into banks.
    Integers can be decimal, hexadecimal (0x prefix), or octal (0 prefix).
    Example output:
        out.hfm - a table of %d-byte (-H) integers giving the end index of each bank
        out0000.hfm - the first bank
        out0001.hfm - the second bank