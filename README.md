# Lol

## TODO

- fix format
- understand crc32, move it to separate thread
- implement compress_chunk



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

1100 1011 0100 1110 1100 1101 1110 0110 0000 0010 0000 0000

```


## Thoughs

```python

master:
  lock MutexWorkerWrite
  createThreads()


  unlock MutexWorkerWrite
  broadcast ConditionWorkerIsAllowedToWrite
  # Not we need to wait intil worker sets bufs

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