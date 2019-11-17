# Lol

## TODO

-   understand crc32, move it to separate thread
-   implement compress_chunk
    -   store len is 0xffff max! split to separate chunks

https://tools.ietf.org/html/rfc1951
https://tools.ietf.org/html/rfc1952

## Windows notes

Docker:

```
docker run -v %cd%:/app -ti --name ubuntu ubuntu bash
apt-get update && apt-get install -y build-essential xxd
```

Use VSCode -> attach to docker

## Gzip notes

```raw
00000000: 1f8b 0800 8c86 d15d 0003 cb4e cde6 0200  .......]...N....
// here is crc32 and size
00000010: a7fd a3e5 0400 0000                      ........

4 bytes of data, here is compressed
cb4e cde6 0200
          |  byte 1  |  byte 2  |  byte 3  |  byte 4 |  byte 5  |
1100 1011  0100 1110  1100 1101  1110 0110  0000 0010  0000 0000
        |_ last block
      |__ Not a huffman code, so it is "10", which means "static huffman"

     | byte 1   |  byte 2    |  byte 3   |  byte 4   |  byte 5  |
10011 011  10010  101 1 0011  011 0 0111  010  00000   000 00000
   k      |    e     |     k     |    0x10    |  256 eob  |  ???

```

## Psoudocode notes

```python

master:
  lock MutexWorkerWrite
  createThreads()

  unlock MutexWorkerWrite
  broadcast ConditionWorkerIsAllowedToWrite

  wait until last block is done


worker:
  pickUpTask()
  doCalculation()

  lock MutexWorkerWrite
  while (ValueMasterIsReadyToRead !== myId):
    wait ConditionWorkerIsAllowedToWrite
  # Here we change output buf
  # Copy whole thing to output buf
  # Increase current pending state
  # Broadcast
  unlock MutexWorkerWrite
```
