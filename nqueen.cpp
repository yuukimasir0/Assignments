#include <iostream>

int n, sum;

void nqueen(int board, int column, int n) {
    int i, j, newboard, checkrow, tmpboard, mask, check;
    i = 1;
    while(i <= n) {
        tmpboard = board;
        mask = 0;
        j = column - 1;
        check = 0;
        while(j > 0) {
            mask = tmpboard / 10;
            mask = mask * 10;
            checkrow = tmpboard - mask;
            if(checkrow == i) {
                check = 1;
            } else if (checkrow == i + (column-j)) {
                check = 1;
            } else if(checkrow == i - (column-j)) {
                check = 1;
            }
            tmpboard = tmpboard / 10;
            j = j - 1;
        }
        if(check == 0) {
            newboard = board * 10 + i;
         if (column == n) {
            printf("%d\n", newboard);
            sum = sum + 1;
        } else
            nqueen(newboard, column + 1, n);
        }
    }
}

int main() {
    sum = 0;
    n = 8;
    nqueen(0, 1, n);
    printf("%d\n", sum);
}