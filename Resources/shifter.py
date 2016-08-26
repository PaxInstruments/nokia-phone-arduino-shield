#!/usr/bin/python

input = 58,93,36
shift = 0
c = 0
w = 0

print "input = ", bin(input[0]), bin(input[1]), bin(input[2])

for (n=0; n<len; ++n) {
    c = input[n] & 0x7f
    c >>= shift
    w = input[n+1] & 0x7f
    w <<= (7-shift)
    shift +=1
    c = c | w
    if (shift == 7) {
        shift = 0x00
        n++
    }
    x = strlen(decode)
    decode[x] = c
    decode[x+1] = 0
}




