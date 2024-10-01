#include <iostream>
#include <mysql.h>
#include <mysqld_error.h>
#include <windows.h>
#include <sstream>
using namespace std;

const char* HOST = "localhost";
const char* USER = "root";
const char* PW = "your_password";
const char* DB = "mydb";

// Seats class to manage seat reservation
class Seats {
private:
    int Seat[5][10];
public:
    Seats() {
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 10; j++) {
                Seat[i][j] = 1;  // 1 means available, 0 means reserved
            }
        }
    }

    int getSeatStatus(int row, int seatNumber) {
        if (row < 1 || row > 5 || seatNumber < 1 || seatNumber > 10) {
            return -1;
        }
        return Seat[row - 1][seatNumber - 1];
    }

    void reserveSeat(int row, int seatNumber) {
        if (row < 1 || row > 5 || seatNumber < 1 || seatNumber > 10) {
            return;
        }
        Seat[row - 1][seatNumber - 1] = 0;
    }

    void display() {
        cout << " ";
        for (int i = 0; i < 10; i++) {
            cout << " " << i + 1;
        }
        cout << endl;

        for (int row = 0; row < 5; row++) {
            cout << row + 1 << " ";
            for (int col = 0; col < 10; col++) {
                if (Seat[row][col] == 1) {
                    cout << "- ";
                } else {
                    cout << "X ";
                }
            }
            cout << "|" << endl;
        }
        cout << "-----------------------" << endl;
    }

    void getDB(MYSQL* conn) {
        string query = "SELECT RowNumber, SeatNumber, Seat FROM Seats";
        if (mysql_query(conn, query.c_str())) {
            cout << "Error: " << mysql_error(conn) << endl;
        }

        MYSQL_RES* result = mysql_store_result(conn);
        if (!result) {
            cout << "Error: " << mysql_error(conn) << endl;
        }

        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            int rowNumber = atoi(row[0]);
            int seatNumber = atoi(row[1]);
            int seatStatus = atoi(row[2]);
            Seat[rowNumber - 1][seatNumber - 1] = seatStatus;
        }
        mysql_free_result(result);
    }
};

// Function to create necessary tables
void createTables(MYSQL* conn) {
    const char* moviesTable = R"(
        CREATE TABLE IF NOT EXISTS Movies (
            movie_id INT AUTO_INCREMENT PRIMARY KEY,
            movie_name VARCHAR(255) NOT NULL,
            movie_genre VARCHAR(100),
            duration_minutes INT,
            show_time DATETIME,
            theater_name VARCHAR(100)
        )
    )";
    mysql_query(conn, moviesTable);

    const char* seatsTable = R"(
        CREATE TABLE IF NOT EXISTS Seats (
            seat_id INT AUTO_INCREMENT PRIMARY KEY,
            movie_id INT,
            row_number INT,
            seat_number INT,
            is_reserved BOOLEAN DEFAULT FALSE,
            FOREIGN KEY (movie_id) REFERENCES Movies(movie_id) ON DELETE CASCADE
        )
    )";
    mysql_query(conn, seatsTable);

    const char* usersTable = R"(
        CREATE TABLE IF NOT EXISTS Users (
            user_id INT AUTO_INCREMENT PRIMARY KEY,
            username VARCHAR(100) NOT NULL,
            email VARCHAR(255) UNIQUE NOT NULL,
            phone_number VARCHAR(15)
        )
    )";
    mysql_query(conn, usersTable);

    const char* bookingsTable = R"(
        CREATE TABLE IF NOT EXISTS Bookings (
            booking_id INT AUTO_INCREMENT PRIMARY KEY,
            user_id INT,
            movie_id INT,
            seat_id INT,
            booking_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE CASCADE,
            FOREIGN KEY (movie_id) REFERENCES Movies(movie_id) ON DELETE CASCADE,
            FOREIGN KEY (seat_id) REFERENCES Seats(seat_id) ON DELETE CASCADE
        )
    )";
    mysql_query(conn, bookingsTable);

    const char* paymentsTable = R"(
        CREATE TABLE IF NOT EXISTS Payments (
            payment_id INT AUTO_INCREMENT PRIMARY KEY,
            booking_id INT,
            user_id INT,
            amount DECIMAL(10, 2) NOT NULL,
            payment_method VARCHAR(50),
            payment_status VARCHAR(50) DEFAULT 'Pending',
            payment_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (booking_id) REFERENCES Bookings(booking_id) ON DELETE CASCADE,
            FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE CASCADE
        )
    )";
	
    mysql_query(conn, paymentsTable);
  const char* createDiscountsTable = R"(
        CREATE TABLE IF NOT EXISTS Discounts (
            discount_id INT AUTO_INCREMENT PRIMARY KEY,
            discount_code VARCHAR(50) UNIQUE NOT NULL,
            discount_amount DECIMAL(5, 2) NOT NULL,
            discount_type VARCHAR(50),
            expiry_date DATETIME,
            minimum_booking_amount DECIMAL(10, 2) DEFAULT 0
        )
    )";
    mysql_query(conn, createDiscountsTable);
	
    const char* theatersTable = R"(
        CREATE TABLE IF NOT EXISTS Theaters (
            theater_id INT AUTO_INCREMENT PRIMARY KEY,
            theater_name VARCHAR(255) NOT NULL,
            location VARCHAR(255),
            total_seats INT
        )
    )";
    mysql_query(conn, theatersTable);

    const char* movieReviewsTable = R"(
        CREATE TABLE IF NOT EXISTS Movie_Reviews (
            review_id INT AUTO_INCREMENT PRIMARY KEY,
            movie_id INT,
            user_id INT,
            rating INT CHECK (rating BETWEEN 1 AND 5),
            review_text TEXT,
            review_date DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (movie_id) REFERENCES Movies(movie_id) ON DELETE CASCADE,
            FOREIGN KEY (user_id) REFERENCES Users(user_id) ON DELETE CASCADE
        )
    )";
    mysql_query(conn, movieReviewsTable);

    const char* adminsTable = R"(
        CREATE TABLE IF NOT EXISTS Admins (
            admin_id INT AUTO_INCREMENT PRIMARY KEY,
            admin_name VARCHAR(100) NOT NULL,
            admin_email VARCHAR(255) UNIQUE NOT NULL,
            password_hash VARCHAR(255) NOT NULL
        )
    )";
    mysql_query(conn, adminsTable);

    const char* showtimesTable = R"(
        CREATE TABLE IF NOT EXISTS Showtimes (
            showtime_id INT AUTO_INCREMENT PRIMARY KEY,
            movie_id INT,
            theater_id INT,
            show_time DATETIME,
            FOREIGN KEY (movie_id) REFERENCES Movies(movie_id) ON DELETE CASCADE,
            FOREIGN KEY (theater_id) REFERENCES Theaters(theater_id) ON DELETE CASCADE
        )
    )";
    mysql_query(conn, showtimesTable);
}

// Main Function
int main() {
    Seats s;
    MYSQL* conn;
    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, HOST, USER, PW, DB, 3306, NULL, 0)) {
        cout << "Error: " << mysql_error(conn) << endl;
    } else {
        cout << "Logged into Database!" << endl;
    }

    // Create all necessary tables
    createTables(conn);

    bool exit = false;
    while (!exit) {
        system("cls");
        cout << "Welcome To Movie Ticket Booking System" << endl;
        cout << "******************************************" << endl;
        cout << "1. Reserve A Ticket" << endl;
        cout << "2. Exit" << endl;
        cout << "Enter Your Choice: ";
        int val;
        cin >> val;

        if (val == 1) {
            // Get available seats
            s.getDB(conn);
            s.display();

            // Input for seat selection
            int row, col;
            cout << "Enter Row (1-5): ";
            cin >> row;
            cout << "Enter Seat Number (1-10): ";
            cin >> col;

            if (row < 1 || row > 5 || col < 1 || col > 10) {
                cout << "Invalid Row or Seat Number!" << endl;
                Sleep(3000);
                continue;
            }

            int seatStatus = s.getSeatStatus(row, col);
            if (seatStatus == -1) {
                cout << "Invalid Row or Seat Number!" << endl;
                Sleep(3000);
                continue;
            }

            if (seatStatus == 0) {
                cout << "Sorry: Seat is already reserved!" << endl;
                Sleep(3000);
                continue;
            }

            s.reserveSeat(row, col);

            stringstream ss;
            ss << "UPDATE Seats SET is_reserved = 1 WHERE row_number = " << row << " AND seat_number = " << col;
            string update = ss.str();

            if (mysql_query(conn, update.c_str())) {
                cout << "Error: " << mysql_error(conn) << endl;
            } else {
                cout << "Seat Reserved Successfully in Row " << row << " and Seat Number " << col << endl;
           
