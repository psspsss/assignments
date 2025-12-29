#include <stdio.h>

int main() {
    int d = 1, m, y;
    int w, i, day;

    printf("Enter Month and Year: ");
    scanf("%d %d", &m, &y);
    if (m < 3) {
        m = m + 12;
        y = y - 1;
    }

    day = (d + (13 * (m + 1)) / 5 + y + (y / 4) - (y / 100) + (y / 400)) % 7;

    w = (day + 6) % 7;

    printf("\nSu Mo Tu We Th Fr Sa\n");

    for (i = 0; i < w; i++) {
        printf("   ");
    }
    for (i = 1; i <= 30; i++) {
        printf("%2d ", i);

        if ((i + w) % 7 == 0)
            printf("\n");
    }

    printf("\n");

    return 0;
}
