# Эксперименты с реализацией gzip в threads

## Сборка и прогон тестов (Linux или Mac)

```sh
./build-and-test.sh
```

## Запуск

```sh
./main.out <input file> <output file>
```

## Комментарии для Windows

Т.к. код завязан на 

## Алгоритм работы



## Plan

1. Try to use static Huffman in compress_chunk
2. Add a fake zero-length "no compression" block to achieve byte-align
3. Checkout CRC32, understand crc merge. If possible, then implement. If hard - then just run CRC in separate thread
4. Try dynamic Huffman in compress_chunk
5. Write good README file

## Links

- https://tools.ietf.org/html/rfc1951
- https://tools.ietf.org/html/rfc1952

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

```raw
00000000  1f 8b 08 00 6d f4 d3 5d  00 03 cb 4e cd 56 c8 86  |....m..]...N.V..|
00000010  60 2e 00 be 64 7a 9b 0c  00 00 00                 |`...dz.....|

cb 4e cd 56 c8 86 60 2e 00

'cb 4e cd 56 c8 86 60 2e 00'.split(' ').
  map(x => parseInt(x, 16).toString(2).padStart(8, '0')).
  map(x => x.split('').reverse().join('')).
  join(' ')

// bits reversed

|- last block
| |- static Huffman
|
110 10011  |  011 10010  |  101 10011  |  011 01010  |  000 10011  |  011
        k        |     e       |      k      |     ' '     |     k       |

00001  |  00 00011 0  |  0111010 0  |  0000000 0
            |     |   0x10      |    eob      |

0b100+256 = 260
 extra=0 len=6

00011 = 4 = distance is 4, extra = 0
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
