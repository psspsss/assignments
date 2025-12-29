#include <stdio.h>

int isLeap(int year) {
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        return 1;
    return 0;
}

int maxDays(int month, int year) {
    if (month == 2)
        return isLeap(year) ? 29 : 28;

    if (month == 4 || month == 6 || month == 9 || month == 11)
        return 30;

    return 31;
}

int dayOfWeek(int d, int m, int y) {
    if (m < 3) {
        m += 12;
        y -= 1;
    }

    return (d + (13 * (m + 1)) / 5 + y + (y / 4)
            - (y / 100) + (y / 400)) % 7;
}

void printCalendar(int month, int year) {
    int days, w, i;

    days = maxDays(month, year);
    w = (dayOfWeek(1, month, year) + 6) % 7;

    printf("\nSu Mo Tu We Th Fr Sa\n");

    for (i = 0; i < w; i++)
        printf("   ");

    for (i = 1; i <= days; i++) {
        printf("%2d ", i);
        if ((i + w) % 7 == 0)
            printf("\n");
    }
    printf("\n");
}

int main() {
    int month, year;

    printf("Enter Month and Year: ");
    scanf("%d %d", &month, &year);

    printCalendar(month, year);

    return 0;
}
