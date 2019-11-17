# Lol

## Windows notes

Docker:
```
docker run -v %cd%:/app -ti --name ubuntu ubuntu bash
apt-get update && apt-get install -y build-essential
```

Use VSCode -> attach to docker

## Gzip notes

```raw
00000000: 1f8b 0800 8c86 d15d 0003 cb4e cde6 0200  .......]...N....
00000010: a7fd a3e5 0400 0000                      ........
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