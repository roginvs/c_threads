# Lol

## Windows notes

Docker:
```
docker run -v %cd%:/app -ti --name ubuntu ubuntu bash
```

Use VSCode -> attach to docker

## Just thoughts

```python

# Worker:
doCalculations()

while True:
  lock masterIsWaitingIndex

  if masterIsWaitingIndex === myId:
     # put values into ready object
     unlock masterIsWaitingIndex
     send workerIsReady
     exit

  wait masterNewWaiting masterIsWaitingIndex (implit unlock masterIsWaitingIndex)
     # now masterIsWaitingIndex is locked








Master:

lock masterIsWaitingIndex
masterIsWaitingIndex = 0
unlock masterIsWaitingIndex

while True:
  broadcast masterNewWaiting

  lock masterIsWaitingIndex

  wait workerIsReady masterIsWaitingIndex
  collectResult(masterIsWaitingIndex)
  masterIsWaitingIndex++

```

```python
# Worker


# Master


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