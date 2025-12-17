# Tama

**Tama** is a database migration tool built in C++23, designed for C++ developers. It is inspired by [Goose](https://pressly.github.io/goose) but aims to provide a native, high-performance solution for managing database schema changes in C++ projects.

## 🚀 Features

Currently, Tama supports the following functionality:

*   **Scaffolding**: Generate timestamped migration SQL files with `up` and `down` annotations.
*   **Configuration**: Loads database connection settings and preferences from a `.env` file.
*   **Modern C++**: Built using C++23 standards.
*   **Build System**: Integrated with CMake.
*   **Dev Environment**: Ready-to-go VSCode debugging configuration.

## 🛠️ Getting Started

### Prerequisites

*   C++23 compatible compiler (GCC, Clang, MSVC)
*   CMake 3.20+
*   SQLite3

### Building

```bash
mkdir build
cd build
cmake ..
make
```

### Usage

#### Initialize a new migration

To create a new migration file in your configured migration directory:

```bash
./tama init migration <migration_name>
```

## ⚙️ Configuration

Tama uses a `.env` file for configuration. Create a `.env` file in the root of your project:

```dotenv
TAMA_DB_MIGRATION_DIR=./migrations
TAMA_DB_ENGINE=sqlite
TAMA_DB_CONNECTION_STRING=tama.db
```

## 🗺️ Roadmap

*   [ ] **SQL Syntax Validation**: Parsing migration files to detect syntax errors (SQLite focus initially).
*   [ ] **Migration State**: Creating a version table in the database to track applied migrations.
*   [ ] **Apply/Rollback**: Commands to run (`up`) and revert (`down`) migrations.

## 🤝 Contributing

Tama is fully open source, and contributions are welcome! Whether it's fixing bugs, adding new database drivers, or improving documentation, feel free to open a pull request.