**IPS specifications.**

Copied from: https://zerosoft.zophar.net/ips.php

IPS format was created a lot of time ago and so the specs for many people are too restrictive for the modern needs.
Well, let see these specs.

- IPS files can patch every file not larger than 2^24-1 bits (2047 Mb)
- Every patch should not be larger than 2^16-1 bits (7.99 Mb)
- An IPS file can hold as many patches he can, assumed that the specified offset and the size of every single patch
  doesn't overflow
  the specified bounds.
  As you should realize it isn't so restrictive... 8 )
  **IPS file format**
  The IPS file structure is made just like that:

| Section            | Size (Bytes) | Description                                                                               |
|--------------------|--------------|-------------------------------------------------------------------------------------------|
| Header             | 5            | The header show ever the same string:  PATCH note that the string is not NULL terminated. |
| Record             | 3+2+Variable | It's the record of a single patch, see below                                              |
| ....               |              | The numbers of record may vary                                                            |
| End Of file marker | 3            | A string (not NULL terminated) saying  EOF                                                |

**IPS Record structure:**

| Section | Size (Bytes) | Description                                                                |
|---------|--------------|----------------------------------------------------------------------------|
| Offset  | 3            | The offset where the patch will be placed in the file to patch             |
| Size    | 2            | The size of the data to put from the specified offset in the patching file |
| Data    | Size         | Contains a number of Size bytes of data to be copied in the file to patch  |

And that's the info you can find around. Now the technical stuff and RLE infos.

**IPS RLE encoding**

The next big step in the comprehension of the IPS format is that some patches can be RLE encoded to save space. The
encoding is very simple but can easily be overlooked if someone is not aware of that. If when you read the size value
of a record this field contains 0 you have a RLE encoded patch. You should read again a 16 bit value to obtain the size
of the RLE patch and then you should read a single byte. This byte must be repeated as many times as the value of the
second 16 bit read. An IPS RLE Record should look like that:

| Section  | Size (Bytes) | Description                                                    |
|----------|--------------|----------------------------------------------------------------|
| Offset   | 3            | Any Value                                                      |
| Size     | 2            | 0                                                              |
| RLE_Size | 2            | Any nonzero value                                              |
| Value    | 1            | This is the value to write RLE_Size times starting from Offset |

**Encoding Of Offset and Size**

For peoples using low level languages a little bytes-swapping is needed to make read right the values of Offset and
Size.
These values are written linearly, just like Pascal and Basic does when handling numerical variables. The problem is
that
low level languages like ASM and C/C++ use the same endianess format of the machine, so every couple of two bytes are
swapped. A 16 bit value is written like this in the IPS: 0x6712, a 24 bit value in this way: 0x671234. If read with a
low
level language they'll throw up: 0x1267 and 0x341267 respectively. Here two C macros that does
the conversion after you read them. bp should be a char array:

```c
#define BYTE3_TO_UINT(bp)   \
(((unsigned int)(bp)[0] << 16) & 0x00FF0000) | \
(((unsigned int)(bp)[1] <<  8) & 0x0000FF00) | \
((unsigned int)(bp)[2] & 0x000000FF)

#define BYTE2_TO_UINT(bp)   \
(((unsigned int)(bp)[0] << 8) & 0xFF00) | \
((unsigned int) (bp)[1] & 0x00FF)
```

16 bit compiler users may find useful to substitute `unsigned int` to `unsigned` `long in` the `BYTE3_TO_UINT` macro...

Real simple once someone has hacked the format for you, isn't it?

**Z.e.r.o**