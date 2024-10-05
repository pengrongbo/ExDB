#include <iostream>
#include <unordered_map>
#include <fstream>
#include <string>
#include <shared_mutex>
#include <vector>

// Storage Module: Responsible for persisting data to and loading data from disk
class Storage {
public:
    // Constructor initializes the storage with the database file name
    explicit Storage(const std::string& dbFileName) : dbFileName_(dbFileName) {}

    // Load data from the database file into an unordered_map
    std::unordered_map<std::string, std::string> load() {
        std::unordered_map<std::string, std::string> db;
        std::ifstream dbFile(dbFileName_);
        std::string key, value;
        while (dbFile >> key >> value) {
            db[key] = value;
        }
        dbFile.close();
        return db;
    }

    // Save the in-memory database to the disk by writing to the database file
    void save(const std::unordered_map<std::string, std::string>& db) {
        std::ofstream dbFile(dbFileName_, std::ios_base::trunc);
        for (const auto& pair : db) {
            dbFile << pair.first << " " << pair.second << "\n";
        }
        dbFile.close();
    }

private:
    std::string dbFileName_;  // Name of the database file
};

// Write-Ahead Logging (WAL) Module: Manages logging of operations for durability and recovery
class WAL {
public:
    // Constructor initializes the WAL with the log file name
    explicit WAL(const std::string& walFileName) : walFileName_(walFileName) {}

    // Log a write (PUT) operation to the WAL
    void logWriteOperation(const std::string& key, const std::string& value) {
        std::ofstream walFile(walFileName_, std::ios_base::app);
        walFile << "PUT " << key << " " << value << "\n";
        walFile.close();
    }

    // Log a delete (DEL) operation to the WAL
    void logDeleteOperation(const std::string& key) {
        std::ofstream walFile(walFileName_, std::ios_base::app);
        walFile << "DEL " << key << "\n";
        walFile.close();
    }

    // Apply the operations recorded in the WAL to the in-memory database
    void applyLog(std::unordered_map<std::string, std::string>& db) {
        std::ifstream walFile(walFileName_);
        std::string operation, key, value;
        while (walFile >> operation >> key) {
            if (operation == "PUT") {
                walFile >> value;
                db[key] = value;
            } else if (operation == "DEL") {
                db.erase(key);
            }
        }
        walFile.close();
    }

    // Clear the WAL after merging logs with the main database
    void clearLog() {
        std::ofstream walFile(walFileName_, std::ios_base::trunc);
        walFile.close();
    }

private:
    std::string walFileName_;  // Name of the WAL file
};

// Core Database Module: Manages data operations, concurrency control, persistence, and logging
class KeyValueDB {
public:
    // Constructor initializes Storage and WAL modules and loads existing data
    KeyValueDB(const std::string& dbFileName, const std::string& walFileName)
        : storage_(dbFileName), wal_(walFileName) {
        // Load persisted data from disk
        db_ = storage_.load();
        // Apply any pending operations from the WAL
        wal_.applyLog(db_);
    }

    // Insert or update a key-value pair
    void put(const std::string& key, const std::string& value) {
        std::unique_lock<std::shared_mutex> lock(mutex_);  // Acquire exclusive lock for writing
        db_[key] = value;                                   // Update in-memory database
        wal_.logWriteOperation(key, value);                 // Log the operation for persistence
    }

    // Retrieve the value associated with a key
    std::string get(const std::string& key) {
        std::shared_lock<std::shared_mutex> lock(mutex_);  // Acquire shared lock for reading
        auto it = db_.find(key);
        if (it != db_.end()) {
            return it->second;
        }
        return "Key not found";
    }

    // Remove a key-value pair
    void remove(const std::string& key) {
        std::unique_lock<std::shared_mutex> lock(mutex_);  // Acquire exclusive lock for writing
        db_.erase(key);                                     // Remove from in-memory database
        wal_.logDeleteOperation(key);                      // Log the operation for persistence
    }

    // Merge the WAL with the main database file and clear the WAL
    void mergeLogs() {
        std::unique_lock<std::shared_mutex> lock(mutex_);  // Acquire exclusive lock for writing
        storage_.save(db_);                                 // Save the current state to disk
        wal_.clearLog();                                    // Clear the WAL after merging
    }

private:
    std::unordered_map<std::string, std::string> db_;    // In-memory database
    Storage storage_;                                     // Storage module for persistence
    WAL wal_;                                             // WAL module for logging
    std::shared_mutex mutex_;                             // Mutex for concurrency control
};

// Test Cases to Demonstrate the KeyValueDB Functionality
int main() {
    // Initialize KeyValueDB with database and WAL file names
    KeyValueDB kvdb("db.txt", "wal.txt");

    // Insert data
    kvdb.put("name", "Alice");
    kvdb.put("age", "30");

    // Retrieve and display data
    std::cout << "name: " << kvdb.get("name") << std::endl;
    std::cout << "age: " << kvdb.get("age") << std::endl;

    // Delete a key-value pair
    kvdb.remove("name");
    std::cout << "name after deletion: " << kvdb.get("name") << std::endl;

    // Merge logs with the main database file
    kvdb.mergeLogs();

    return 0;
}
