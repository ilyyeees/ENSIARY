# ENSIARY Library Management System

## Project Overview

ENSIARY is a comprehensive library management system built with Qt, designed to streamline the management of books, students, and borrowing operations in an educational institution. The application provides a robust, user-friendly interface that supports multiple layers of functionality, from basic book cataloging to advanced user management and activity tracking.

The system is built on a multi-tier architecture that separates the user interface, business logic, and data access layers. This design ensures maintainability, extensibility, and proper separation of concerns. ENSIARY uses a local SQLite database for persistent storage of all data, making it portable and easy to deploy.

## Core Features

### 1. Authentication and Multi-level Access

- **Admin Authentication**: Secure login system for library administrators
- **Root Admin Privileges**: Special privileges for the root administrator, including user management
- **Action Logging**: Comprehensive logging of all administrative actions for accountability and audit

### 2. Book Management

- **Comprehensive Book Cataloging**: Store detailed information about books including title, author, ISBN, and number of copies
- **Multi-copy Support**: Track multiple copies of the same book independently
- **Real-time Availability Tracking**: See which books are available, borrowed, or fully checked out
- **Advanced Search Functionality**: Search books by title, author, or ISBN with real-time filtering
- **Sorting Options**: Sort book lists by various criteria including title, author, and availability
- **Borrowing Workflow**: Streamlined process for checking books in and out
- **Borrower Tracking**: See who has borrowed which books and when
- **Batch Operations**: Perform operations on multiple books or copies

### 3. Student Management

- **Student Registration**: Add and manage student records with comprehensive details
- **Student Profiles**: Store names, IDs, dates of birth, years, groups, and email addresses
- **CSV Bulk Import**: Ability to import student records in bulk from CSV files
- **Borrow History**: Track each student's complete borrowing history
- **Active Borrows**: See which books a student currently has borrowed
- **Year and Group Management**: Organize students by academic year and class group
- **Advanced Filtering**: Filter students by various criteria including year and group
- **Search Functionality**: Quickly find students by name, ID, or email

### 4. Admin Management

- **Admin Registration**: Add and manage library administrators
- **Privilege Control**: Differentiate between regular admins and root admin
- **Activity Logs**: Track all actions performed by each administrator
- **Admin Profiles**: View and manage admin account details

### 5. User Interface Features

- **Modern UI Design**: Clean, intuitive interface with responsive controls
- **Tabbed Interface**: Organized access to different functional areas
- **Login Screen**: Secure gateway to the application
- **Status Updates**: Real-time status messages and alerts
- **Context-sensitive Help**: Detailed help information based on current context
- **Animated Transitions**: Smooth animations for improved user experience
- **Splash Screen**: Professional application startup with branding
- **Icon and Branding**: Consistent visual identity throughout the application

## Technical Implementation

### Architecture

ENSIARY follows a layered architecture:

1. **Presentation Layer** (`MainWindow` class)
   - Handles all UI components and user interactions
   - Manages views, forms, and visual elements
   - Delegates business operations to the service layer

2. **Service Layer** (`Library` class)
   - Implements business logic and operations
   - Manages authentication and authorization
   - Coordinates between UI and data access

3. **Data Access Layer** (`DatabaseManager` class)
   - Handles all database operations
   - Provides CRUD operations for all entities
   - Manages data persistence and retrieval

4. **Domain Model** (Various entity classes)
   - `Book`: Represents book information and status
   - `User`: Base class for system users
   - `Student`: Extends User with student-specific attributes
   - `Admin`: Extends User with admin capabilities
   - Various supporting data structures

### Key Components

#### Database Management

The `DatabaseManager` class provides a comprehensive interface to the SQLite database:

- **Table Creation**: Automatically creates the required database schema if not present
- **Data Operations**: Handles all CRUD operations for books, students, and admins
- **Transaction Support**: Ensures data integrity during complex operations
- **Query Optimization**: Efficiently retrieves and filters data

#### Library Management

The `Library` class serves as the central service layer:

- **Book Operations**: Adding, removing, borrowing, and returning books
- **Student Management**: Registration, updates, and history tracking
- **Admin Authentication**: User validation and session management
- **Cache Management**: Maintains optimized in-memory representation of data

#### User Interface

The `MainWindow` class implements a comprehensive UI with:

- **Stacked Layout**: Switches between login and main interface
- **Tabbed Interface**: Organizes functionality into logical sections
- **Dynamic Forms**: Context-sensitive input and display areas
- **Table Views**: Sortable, filterable data grids for various entities
- **Search and Filter**: Real-time filtering across multiple criteria
- **Status Updates**: Animated status messages and notifications

### Data Models

#### Book Model

Books are represented by the `Book` class, which includes:

- Basic information: title, author, ISBN
- Inventory tracking: total copies, borrowed copies
- Borrower tracking: who has borrowed which copies
- Status management: availability and borrowing state

#### User Model Hierarchy

- `User` (Base class)
  - Common attributes: name, ID, date of birth, year, group, email
  - Basic operations common to all users

- `Student` (Extends User)
  - Borrowing history tracking
  - Academic information: year and group
  - Active borrow management

- `Admin` (Extends User)
  - Authentication credentials
  - Action logging capabilities
  - Administrative privileges

## Database Schema

ENSIARY uses an SQLite database with the following tables:

1. **Books**: Stores main book information
   - id (Primary Key)
   - title
   - author
   - isbn (Unique)
   - total_copies

2. **BookCopies**: Tracks individual copies of books
   - copy_id (Primary Key)
   - book_id (Foreign Key to Books)
   - borrowed (Boolean)
   - borrower_name (Nullable)
   - borrower_id (Nullable, Foreign Key to Students)
   - borrower_year (Nullable)
   - borrower_group (Nullable)
   - borrowed_date (Nullable)

3. **Students**: Stores student information
   - id (Primary Key)
   - name
   - dob (Date of Birth)
   - year
   - group
   - email

4. **BorrowHistory**: Tracks borrowing history
   - id (Primary Key)
   - student_id (Foreign Key to Students)
   - book_isbn
   - book_title
   - book_author
   - borrow_date
   - return_date (Nullable, empty if not returned)

5. **Admins**: Stores administrator accounts
   - id (Primary Key)
   - name
   - password_hash (Encrypted)
   - is_root (Boolean)

6. **AdminActions**: Logs all administrator actions
   - id (Primary Key)
   - admin_id (Foreign Key to Admins)
   - admin_name
   - action_type
   - action_details
   - timestamp

## Setup and Dependencies

### System Requirements

- Operating System: Windows, macOS, or Linux
- Disk Space: 50 MB minimum
- Memory: 256 MB minimum (512 MB recommended)
- Display: 1024x768 minimum resolution

### Dependencies

- Qt 6.x (Core, GUI, Widgets, SQL, SVG)
- SQLite 3
- C++17 compatible compiler

### Development Environment Setup

1. Install Qt 6.x and Qt Creator
2. Clone the repository
3. Open ENSIARY.pro in Qt Creator
4. Configure the project for your target platform
5. Build the project

### Compilation

```bash
# Using qmake directly
qmake ENSIARY.pro
make

# Or using Qt Creator
# Open ENSIARY.pro and click Build
```

## Usage Guide

### Initial Setup

1. Launch the application
2. On first run, the database will be initialized
3. If no students exist, they will be imported from students.csv (if available)
4. Login with default credentials:
   - Username: root
   - Password: root

### Book Management

#### Adding Books
1. Navigate to the "Book Management" tab
2. Fill in the book details (title, author, ISBN, copies)
3. Click "Add Book"

#### Borrowing Books
1. Select a book from the "All Books" or "Available Books" tab
2. Click "Borrow"
3. Select a student from the dialog
4. Confirm the borrowing operation

#### Returning Books
1. Select a borrowed book from the "All Books" or "Borrowed Books" tab
2. Click "Return"
3. If multiple students have borrowed the same book, select which copy to return
4. Confirm the return operation

#### Deleting Books
1. Select a book from any tab
2. Click "Delete"
3. Confirm the deletion

### Student Management

#### Adding Students
1. Navigate to the "Student Management" tab
2. Fill in the student details (name, ID, email, date of birth, year, group)
3. Click "Add Student"

#### Editing Students
1. Select a student from the table
2. The student's details will populate the form
3. Modify the details as needed
4. Click "Edit Student"

#### Viewing Borrow History
1. Select a student from the table
2. Click the "Borrow History" tab to view that student's complete borrowing history

### Admin Management

#### Adding Admins (Root Admin Only)
1. Navigate to the "Admin" tab
2. Fill in the new admin's details
3. Click "Add Admin"

#### Viewing Action Logs
1. Navigate to the "Admin" tab
2. View the complete log of administrative actions

## CSV Import Format

The system supports importing students from a CSV file named "students.csv" located in the application directory. The format should be:

```
Name, ID, Group-Section, Email
```

Example:
```
John Doe, 12345, 1-2, john.doe@example.com
Jane Smith, 67890, 2-3, jane.smith@example.com
```

Notes:
- All imported students are initially set as first-year students
- The year is derived from the first number in the Group-Section field
- The group is derived from the second number in the Group-Section field
- Date of birth is initially empty and can be edited later

## Implementation Notes

### Security Considerations

- Passwords are stored in plaintext (in the current version) - this should be improved in production
- The application uses local authentication only
- No network communication is implemented
- Data integrity is maintained through database transactions

### Performance Optimizations

- Book data is cached in memory for faster access
- Tables are updated only when needed to improve UI responsiveness
- Search and filtering operations are performed in real-time

### Limitations and Future Improvements

- Password hashing should be implemented for production use
- Network support for distributed deployments
- Barcode/RFID integration for physical book tracking
- Report generation and export capabilities
- More comprehensive book metadata support
- Enhanced fine and due date management

## License

All rights reserved. This software is proprietary and confidential.

## Contact

For support or inquiries, please contact the development team.

---

This documentation is current as of March 2024. 