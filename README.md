# ExDB

`ExDB` is a simple key-value database implemented in C++ that supports persistent storage and Write-Ahead Logging (WAL) for data durability and recovery. It uses a modular design with concurrency control for thread-safe operations. The database is ideal for small to medium-sized projects where lightweight and efficient key-value storage is needed.

## Features

- **Persistent Storage**: Data is saved to disk (`db.txt`) and can be restored on program restart.
- **Write-Ahead Logging (WAL)**: Operations are logged before execution to ensure durability and enable recovery.
- **Thread-Safe**: Supports concurrent reads with exclusive access for writes using `std::shared_mutex`.
- **Log Merging**: Logs can be merged into the main database file to reduce log file size and optimize performance.

## Requirements

To compile and run the project, you need:
- A C++ compiler (such as `g++`).
- C++11 or later.

## Files

- `ExDB.cpp`: The main C++ source code that implements the key-value database.
- `db.txt`: The database file where key-value pairs are persisted.
- `wal.txt`: The write-ahead log file where all operations are logged for recovery.

## Compilation

Use `g++` or any other C++ compiler to compile the source code.

```bash
g++ -std=c++11 ExDB.cpp -o ExDB
```

This will create an executable named `ExDB`.

## Usage

Once the program is compiled, you can run the executable:

```bash
./ExDB
```

The database will automatically load any existing data from `db.txt` and apply any operations from `wal.txt`.

### Basic Operations

The following operations are supported by the `ExDB` program:

1. **Insert or Update (PUT)**
   - Inserts a new key-value pair or updates an existing key with a new value.
   - The operation is logged to `wal.txt` before it is executed to ensure durability.
   
2. **Retrieve (GET)**
   - Retrieves the value associated with a given key.
   - The operation is thread-safe, allowing multiple concurrent read operations.
   
3. **Delete (DEL)**
   - Removes a key-value pair from the database.
   - The delete operation is logged to `wal.txt` to ensure it can be recovered in case of failure.

4. **Merge Logs**
   - Merges the operations recorded in `wal.txt` into the main database file (`db.txt`).
   - After merging, the `wal.txt` file is cleared to reduce file size and improve performance.

### Example Operations

```cpp
ExDB kvdb("db.txt", "wal.txt");

// Insert or update data
kvdb.put("name", "Alice");
kvdb.put("age", "30");

// Retrieve data
std::cout << "name: " << kvdb.get("name") << std::endl;
std::cout << "age: " << kvdb.get("age") << std::endl;

// Delete data
kvdb.remove("name");
std::cout << "name after deletion: " << kvdb.get("name") << std::endl;

// Merge logs with the main database
kvdb.mergeLogs();
```

### Output Example

```bash
name: Alice
age: 30
name after deletion: Key not found
```

## How the Code Works

### 1. `ExDB.cpp`

The main C++ file that contains all the logic for the key-value database.

- **Modules**:
  - `Storage`: Handles reading from and writing to the database file (`db.txt`).
  - `WAL`: Manages write-ahead logging, storing operations in `wal.txt` before they are executed.
  - `ExDB`: The core database logic, which integrates `Storage` and `WAL`, and provides methods like `put()`, `get()`, `remove()`, and `mergeLogs()`.

### 2. `db.txt`

- Stores the actual key-value pairs in plain text.
- Each line represents a key-value pair in the format:
  ```
  key value
  ```
- This file is read at the start of the program and written to during log merges.

### 3. `wal.txt`

- Stores logged operations (PUT or DEL) to ensure durability.
- Format of the file:
  ```
  PUT key value
  DEL key
  ```
- These logs are replayed during program startup to ensure that any operations not yet merged into `db.txt` are applied.

## Functions Overview

### `ExDB::put(const std::string& key, const std::string& value)`
- Inserts a new key-value pair or updates an existing one.
- Logs the operation to `wal.txt` and applies it to the in-memory database.

### `ExDB::get(const std::string& key)`
- Retrieves the value associated with a key.
- If the key is not found, returns `"Key not found"`.

### `ExDB::remove(const std::string& key)`
- Deletes a key-value pair from the in-memory database.
- Logs the delete operation to `wal.txt`.

### `ExDB::mergeLogs()`
- Merges the operations recorded in `wal.txt` into `db.txt`.
- Clears the log file after merging to optimize disk usage.

## Logging and Recovery

### Write-Ahead Logging (WAL)
- Every operation (insert/update/delete) is logged to `wal.txt` before being applied.
- This ensures that even if the program crashes, all operations can be replayed during startup to bring the database to a consistent state.

### Log Merging
- To prevent `wal.txt` from growing indefinitely, the `mergeLogs()` function writes all current data to `db.txt` and clears the log.
- It's recommended to call this function periodically to maintain efficiency.

## Thread Safety

- The database uses `std::shared_mutex` to ensure that multiple threads can read data concurrently while writes and deletes are locked to prevent data corruption.

## Future Improvements

- **Range Queries**: Add support for efficient range queries using data structures like B-trees.
- **Data Compression**: Implement data compression for the database and log files to reduce disk space usage.
- **Transactions**: Add transaction support for atomic multi-operation executions.

## License

This project is open-source and available under the [MIT License](LICENSE).
