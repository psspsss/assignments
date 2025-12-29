#include <stdio.h>

int main() {
    int d, m, y;
    int day;

    printf("Enter date (dd mm yyyy): ");
    scanf("%d %d %d", &d, &m, &y);

    if (m < 3) {
        m = m + 12;
        y = y - 1;
    }
    day = (d + (13 * (m + 1)) / 5 + y + (y / 4) - (y / 100) + (y / 400)) % 7;

    printf("Day of the week: ");

    if (day == 0) printf("Saturday\n");
    if (day == 1) printf("Sunday\n");
    if (day == 2) printf("Monday\n");
    if (day == 3) printf("Tuesday\n");
    if (day == 4) printf("Wednesday\n");
    if (day == 5) printf("Thursday\n");
    if (day == 6) printf("Friday\n");

    return 0;
}
