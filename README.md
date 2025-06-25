# 📚 ENSIARY - Library Management System

<div align="center">

![ENSIARY Logo](https://img.shields.io/badge/ENSIARY-Library%20Management-4A90E2?style=for-the-badge&logo=book&logoColor=white)
![Qt](https://img.shields.io/badge/Qt-6.8-41CD52?style=for-the-badge&logo=qt&logoColor=white)
![C++](https://img.shields.io/badge/C++-20-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)

**A modern library management system built with Qt6 & C++**

*By ILYES ABBAS*

</div>

---

## ✨ What is ENSIARY?

ENSIARY is a powerful yet user-friendly library management system that helps librarians manage books, users, loans, and reservations efficiently. Built with modern C++ and Qt, it offers a clean graphical interface for all your library needs.

## 🚀 Key Features

### 📖 **Manage Your Collection**
- Add books, articles, theses, and digital content
- Search and filter resources easily
- Track availability and location

### 👥 **User Management**
- Support for students, faculty, staff, and guests
- Different borrowing limits for each user type
- Complete user profiles and history

### 🔄 **Loan System**
- Easy borrowing and returning
- Automatic due date calculation
- Loan renewals and overdue tracking

### 📅 **Reservations**
- Reserve books that are currently borrowed
- Priority queue system
- Automatic notifications when available

## 💻 Getting Started

### Requirements
- **Qt 6.x** framework
- **C++20** compatible compiler
- **Windows**, **macOS**, or **Linux**

### Quick Setup

1. **Install Qt 6** from [qt.io](https://www.qt.io/download)
2. **Open the project** in Qt Creator
3. **Build and run** (press `Ctrl+R`)

That's it! The app will create sample data on first launch.

### Command Line Build
```bash
# Navigate to project folder
cd ENSIARY

# Build
qmake ENSIARY.pro
make

# Run
./ENSIARY
```

## 📱 How to Use

### Adding Books
1. Go to **Resource Management** tab
2. Click **"Add Resource"**
3. Fill in book details
4. Save!

### Managing Users
1. Open **User Management** tab
2. Click **"Add User"**
3. Enter user information
4. Set user type (Student, Faculty, etc.)

### Borrowing Books
1. Find the book in the catalog
2. Click **"Borrow"**
3. Select the borrower
4. Confirm the loan

### Returning Books
1. Go to **Loan Management**
2. Select the loan to return
3. Click **"Return"**

## 🎯 Project Structure

```
ENSIARY/
├── 📄 ENSIARY.pro           # Project file
├── 📖 README.md             # This file
├── 💻 src/                  # Source code
│   ├── main.cpp            # App entry point
│   ├── mainwindow.*        # Main window
│   ├── models/             # Data models
│   │   ├── resource.*      # Books, articles, etc.
│   │   ├── user.*          # User management
│   │   └── loan.*          # Loan tracking
│   └── services/           # Business logic
│       ├── library_manager.*    # Main logic
│       └── persistence_service.* # Data storage
└── 📊 data/                 # JSON data files
    ├── resources.json      # Book catalog
    ├── users.json          # User database
    └── loans.json          # Loan records
```

## 🔧 Technical Highlights

- **Modern C++20** with smart pointers and STL containers
- **Qt6 Framework** for beautiful, native GUIs
- **JSON Storage** for human-readable data files
- **Object-Oriented Design** with inheritance and polymorphism
- **Cross-Platform** support (Windows, macOS, Linux)

## 🎨 Features in Detail

### Smart Search
- Search by title, author, or ISBN
- Filter by category, status, or user type
- Real-time results as you type

### Flexible Loan Rules
- Different loan periods for different user types
- Students: 2 weeks, Faculty: 4 weeks
- Automatic renewal (if no reservations)
- Grace period for overdue items

### Priority System
- Faculty gets priority for reservations
- Students and staff follow first-come-first-served
- Automatic notifications when books become available

## 🛠️ Development

Built using modern software engineering practices:
- **Qt Signal/Slot System** for responsive UI
- **JSON Data Persistence** for easy data management
- **Exception Safety** with proper error handling

## 📝 License

Educational project by **ILYES ABBAS**

## 🤝 Contributing

Feel free to fork, improve, and submit pull requests!

---

<div align="center">

**ENSIARY** - A Better Library For ENSIA

*Built using Qt6 and C++20*

</div>
