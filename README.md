# Lol

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
