import struct
import sys

# bytes per single LRU entry
blocksize=8

flru = open(sys.argv[1], 'rb')

L = []

while True:
        s1 = flru.read(blocksize)
        if s1=="":
                break;
        # InnoDB uses big-endian format for storage
        val = struct.unpack('>II', s1)
        if (val[0]>0) or (val[1]>0):
                L.append(val)

print "Length:", len(L)
L.sort()

flru.close()

fsorted = open(sys.argv[1], 'wb')
for item in L:
        # InnoDB uses big-endian format for storage
        pk=struct.pack('>II', item[0],item[1])
        fsorted.write(pk)

pk=struct.pack('>II',0,0);

for i in range(0, 16*1024 / 8):
        fsorted.write(pk)

fsorted.close()

print "Done!"
