## `iofox::packed_arg<T>`
- Свойство экзекьютора в терминах `Unified Executors Proposal (p0443r13)`.
- Представляющее запакованный аргумент типа T.
- Для хранения в `iofox::packed_executor`.
- Хранимый тип `T` должен удовлетворять `std::default_initializable`.
- Свойство екзекьютора, которое может быть установлено на `iofox::packed_executor` или `iofox::any_executor`, при условии что он содержит `iofox::packed_executor`.
- Свойство исполнителя.
- Может быть использовано в выражениях: `require`, `prefer`, `query`.

> Хранимый тип `T` должен удовлетворять `std::default_initializable`, так как он может быть использован в полиморфном исполнителе.

Использование:
```cpp
executor = boost::asio::require(executor, iofox::packed_arg<int *>(&ptr));
```
```cpp
int * ptr = boost::asio::query(executor, iofox::packed_arg<int *>());
```


## `iofox::packed_executor<T, Args...>`
- fixed-size collection of heterogeneous values.

## `iofox::any_executor`
Description.
