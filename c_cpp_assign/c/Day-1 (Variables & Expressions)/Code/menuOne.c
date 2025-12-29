#include <stdio.h>

int main(){
	int choice;
	do {
		printf("Enter the choice: 1 Edge/2 Python/3 Java/else quit");
		scanf("%d", &choice);
		switch(choice){
			case 1:
				printf("You have chosen Edge\n");
				//later when functions are introduced
				break;
			case 2:
				printf("You have chosen Python\n");
				//later when functions are introduced
				break;
			case 3:
				printf("You have chosen Java\n");
				//later when functions are introduced
				break;
			default:
				printf("No such option Available\n");
				return 1;
		}
	}while(1);//Always true here


}
