# Эксперименты с реализацией gzip в threads

## Сборка и прогон тестов (Linux или Mac)

```sh
./build-and-test.sh
```

Результат прогона тестов для последних коммитов
можно посмотреть в [Github Actions](./../../actions)

![build status badge](https://github.com/roginvs/c_threads/workflows/ci_test/badge.svg)

## Запуск

```sh
./main.out <input file> <output file>
```

## Комментарии для Windows

Т.к. код завязан на `pthread` и `mmap`, то самое просто это запускать код в Docker:

```cmd
docker run -v %cd%:/app -ti --name ubuntu ubuntu bash
apt-get update && apt-get install -y build-essential xxd
```

Можно использовать VSCode remote developing extensions и подключиться прямо в контейнер

## Алгоритм работы

Основная функция `gzip` из файла `gzip.c` принимает следующие аргументы:

```C
void gzip(
  /** Указатель на входные данные */
  uint8_t *input_buf,

  /** Размер входных данных */
  int32_t input_buf_len,

  /** Количество потоков */
  int32_t threads_count,

  /** Коллбэк для записи кодированных данных */
  write_handler write,

  /** Произвольный указатель, он будет передан в коллбэк */
  void *write_user_data
)

/** Тип колбека на запись */
typedef void (*write_handler)(uint8_t *buf, uint32_t len, void *user_data);
```

Функция вызывает коллбэк на запись (несколько раз) и возвращает управление когда всё завершено.

Внутри функции создается `threads_count` потоков. Каждый поток самостоятельно разбирает номер блока для себя.

Когда поток закончил, то он ожидает до тех пор, как `worker_is_allowed_to_write` станет равным его номеру блока.

Как только это произошло, поток вызывает коллбэк, увеличивает `worker_is_allowed_to_write` и рассылает всем остальным потокам уведомление что переменная поменялась. После этого поток повторяет свой цикл пока есть несделанные блоки.

Мастер так же ожидает `worker_is_allowed_to_write`. Как только последний блок готов, мастер завершает потоки и дописывает footer.

## CRC32

Реализован подсчёт CRC32 в воркерах. Детали можно посмотреть в файле `crc32.c`

```C
/** Выполняет однопоточный подсчёт CRC */
uint32_t crc32(const uint8_t *data, uint32_t length)
```

Для многопоточного crc нужно использовать функции:

```C
/** Считает crc для блока с учётом положения блока. Возвращает промежуточный результат */
uint32_t crc32_partial_block(
  /** Указатель на блок */
  const uint8_t *data,
  /** Длинна блока */
  uint32_t block_length,
  /**
   * Количество байт до этого блока.
   * Используются только значения 0, 1, 2, 3, >4. Т.е. любое число больше 4 даст тот же результат */
  uint32_t bytes_before,
  /**
   * Количество байт после этого блока.
   * */
  uint32_t bytes_after
)

/** Соединить данные от блоков. Не обязятельно в исходном порядке */
uint32_t crc32_block_combine(uint32_t crc1, uint32_t crc2)

/** Финальное преобразование */
uint32_t crc32_finallize(uint32_t crc)
```

Для каждого блока данных нужно подсчитать значение `crc32_partial_block`. Затем значения каждого блока нужно
собрать воедино в любом порядке с `crc32_block_combine`. Можно использовать `CRC32_INITIAL` для удобства.
И в конце финализировать значение с `crc32_finallize`.

Примечание: параллелльный CRC не работает на данных меньше 4 байта. Это баг.
Для данных меньше 4-й байт можно подсчитать CRC через обычный `crc32`

## Сжатие

На текущий момент сжатие как таковое не реализованно, используется deflate без сжатия. Предполагается что функция `compress_chunk` будет определять как именно сохранять текущий блок.

## Граничные случаи на больших данных

Код нуждается в проверке на граничные случаи. Что будет если входной файл размером 0xFFFFFFFF? Или на байт больше? Или на два байта больше?

## Ссылки

- https://tools.ietf.org/html/rfc1951
- https://tools.ietf.org/html/rfc1952
- http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
- http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html
- https://nicst.de/crc.pdf

## Всякие заметки по формату gzip

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

## Заметки на псевдокоде по алгоритму синхронизации потоков

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
