
# Motiviation

Due to incompatibilities between LLVM-MOS and cc65, we simply build certain programs as a giant binary blob
and the import the blobs as binary into the assembly.

This lets us use the common libraries and functions that are targetting ca65, albeit a bit painfully.
