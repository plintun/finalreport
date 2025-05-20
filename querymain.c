
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
// Check if a datetime string is in the correct format
int check_datetime(const char* datetime) {
    return sscanf(datetime, "%4d-%2d-%2d %2d:%2d:%2d",
        &(int){0}, &(int){0}, &(int){0},
        &(int){0}, &(int){0}, &(int){0}) == 6;
}

// Display data rows for a given time range
void show_data_for_range(MYSQL *conn, const char* start, const char* end) {
    char query[300];
    snprintf(query, sizeof(query),
        "SELECT temperature, humidity, datainserttime "
        "FROM results WHERE datainserttime BETWEEN '%s' AND '%s'",
        start, end);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Query error: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row;

    if (res == NULL || mysql_num_rows(res) == 0) {
        printf("No data found for the given time range.\n");
    } else {
        printf("\n--- Data from %s to %s ---\n", start, end);
        printf("Temperature | Humidity | Timestamp\n");
        while ((row = mysql_fetch_row(res))) {
            printf("%s°F | %s%% | %s\n",
                row[0] ? row[0] : "NULL",
                row[1] ? row[1] : "NULL",
                row[2] ? row[2] : "NULL");
        }
    }

    mysql_free_result(res);
}

// Show min, max, and average values for a time range
void show_stats_for_range(MYSQL *conn, const char* start, const char* end) {
    char query[512];
    snprintf(query, sizeof(query),
        "SELECT "
        "MIN(temperature), MAX(temperature), AVG(temperature), "
        "MIN(humidity), MAX(humidity), AVG(humidity) "
        "FROM results WHERE datainserttime BETWEEN '%s' AND '%s'",
        start, end);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Query error (stats): %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(res);

    if (!row || !row[0] || !row[1] || !row[2] || !row[3] || !row[4] || !row[5]) {
        printf("\nNot enough data for statistics.\n");
    } else {
        printf("\n--- Summary Statistics ---\n");
        printf("Temperature (°F): Min: %.2f, Max: %.2f, Avg: %.2f\n",
            atof(row[0]), atof(row[1]), atof(row[2]));
        printf("Humidity (%%): Min: %.2f, Max: %.2f, Avg: %.2f\n",
            atof(row[3]), atof(row[4]), atof(row[5]));
    }

    mysql_free_result(res);
}
void displayMenu() {
    printf("\n=== Sensor Data Query Menu ===\n");
    printf("1. Show 3-day temperature & humidity average\n");
    printf("2. Show all sensor readings\n");
    printf("3. Show min and max temperature and humidity\n");
    printf("4. Show temperature and humidity data for date and time and average humidity and temperature\n");
    printf("5. Exit\n");
    printf("Enter choice: ");
}
int main(void) {
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;



    char *server = "localhost";
    char *user = "john";
    char *password = "password";
    char *database = "tempdata";

    // Initialize MySQL connection
    conn = mysql_init(NULL);
    /* Connect to database */
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        fprintf(stderr, "Connection error: %s\n", mysql_error(conn));
        exit(1);
    }

    int choice;
    while (1) {
        displayMenu();
        scanf("%d", &choice);
// Show average temperature and humidity
        if (choice == 1) {
            const char *query = "SELECT AVG(temperature), AVG(humidity) FROM results";
            if (mysql_query(conn, query)) {
                fprintf(stderr, "Query error: %s\n", mysql_error(conn));
                continue;
            }

            res = mysql_store_result(conn);
            if ((row = mysql_fetch_row(res))) {
                if (row[0] == NULL || row[1] == NULL) {
                    printf("Not enough data for 3-day average.\n");
                } else {
                    printf("3-day average temp: %.2f°F, humidity: %.2f%%\n", atof(row[0]), atof(row[1]));
                }
            } else {
                printf("No data returned.\n");
            }
            mysql_free_result(res);
        }
// Display all sensor readings and export to CSV
        else if (choice == 2) {
            const char *query = "SELECT * FROM results";
            if (mysql_query(conn, query)) {
                fprintf(stderr, "Query error: %s\n", mysql_error(conn));
                continue;
            }

            res = mysql_store_result(conn);
            if (res == NULL) {
                fprintf(stderr, "Result error: %s\n", mysql_error(conn));
                continue;
            }

            unsigned long total = mysql_num_rows(res);
            printf("Total records: %lu\n", total);
            printf("Temp (°F) | Humidity (%%) | Timestamp\n");
            printf("--------------------------------------\n");

            FILE *csv = fopen("results.csv", "w");
            if (csv == NULL) {
                perror("Could not open results.csv for writing");
                mysql_free_result(res);
                continue;
            }

            // Write CSV header
            fprintf(csv, "Temperature,Humidity,Timestamp\n");

            while ((row = mysql_fetch_row(res))) {
                const char *temp = row[1] ? row[1] : "";
                const char *hum = row[2] ? row[2] : "";
                const char *time = row[3] ? row[3] : "";

                // Print to terminal
                printf("%s°F | %s%% | %s\n", temp, hum, time);

                // Write to CSV file
                fprintf(csv, "%s,%s,%s\n", temp, hum, time);
            }

            fclose(csv);
            mysql_free_result(res);
        }



//Show min and max temperature and humidity
        else if (choice == 3) {
            const char *query = "SELECT MIN(temperature), MAX(temperature), MIN(humidity), MAX(humidity) FROM results";
            if (mysql_query(conn, query)) {
                fprintf(stderr, "Query error: %s\n", mysql_error(conn));
                continue;
            }

            res = mysql_store_result(conn);
            if ((row = mysql_fetch_row(res))) {
                if (row[0] == NULL || row[1] == NULL || row[2] == NULL || row[3] == NULL) {
                    printf("Not enough data for min/max values.\n");
                } else {
                    printf("Min Temp: %.2f°F, Max Temp: %.2f°F\n", atof(row[0]), atof(row[1]));
                    printf("Min Humidity: %.2f%%, Max Humidity: %.2f%%\n", atof(row[2]), atof(row[3]));
                }
            } else {
                printf("No data returned.\n");
            }
            mysql_free_result(res);
        }
        // Show data for a specific time range and average temperature and humidity
        else if (choice == 4) {
            char start[20], end[20];

            printf("Enter start datetime (YYYY-MM-DD HH:MM:SS): ");
            scanf(" %[^\n]", start);
            getchar();

            printf("Enter end datetime (YYYY-MM-DD HH:MM:SS): ");
            scanf(" %[^\n]", end);
            getchar();

            if (!check_datetime(start) || !check_datetime(end)) {
                printf("Invalid date format. Please use YYYY-MM-DD HH:MM:SS\n");
                continue;
            }

            show_data_for_range(conn, start, end);
            show_stats_for_range(conn, start, end);
        }

