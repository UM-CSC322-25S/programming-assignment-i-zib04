//-----------------------------ENJOY GRADING! THANKS <3------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOATS 120
#define MAX_NAME_STRING 128
#define MAX_PLACE_STRING 10
#define MAX_TRAILER_STRING 10

typedef char NameString[MAX_NAME_STRING];
typedef char PlaceString[MAX_PLACE_STRING];
typedef char TrailorString[MAX_TRAILER_STRING];

typedef enum {
    slip,
    land,
    trailor,
    storage,
    no_place
} PlaceType;

typedef union {
    int SlipData;
    char LandData;
    TrailorString TrailorData;
    int StorageData;
} BoatUnionType;

typedef struct {
    NameString BoatName;
    int length;
    PlaceType Place;
    BoatUnionType ExtraInfo;
    float MoneyOwed;
} BoatRecordType;
//-----------------------------------------------------------------------------
// Functions
void *Malloc(size_t Size);
int CompareBoats(const void *A, const void *B);
PlaceType StringToPlaceType(char *PlaceString);
char *PlaceToString(PlaceType Place);
void LoadBoats(BoatRecordType *BoatRecord[MAX_BOATS], char *filename, int *totalBoats);
void AddBoat(BoatRecordType *BoatRecord[MAX_BOATS], int *totalBoats, char *boatData);
void RemoveBoat(BoatRecordType *BoatRecord[MAX_BOATS], int *totalBoats, char *boatName);
void AcceptPayment(BoatRecordType *BoatRecord[MAX_BOATS], int index, float Pay);
void MonthlyUpdate(BoatRecordType *BoatRecord[MAX_BOATS], int totalBoats);
void PrintInventory(BoatRecordType *BoatRecord[MAX_BOATS], int totalBoats);
void SaveBoats(BoatRecordType *BoatRecord[MAX_BOATS], char *filename, int totalBoats);
//-----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    // Variables
    FILE *BoatFile;
    BoatRecordType *BoatRecord[MAX_BOATS] = {};
    int totalBoats = 0;
    NameString Name = "";
    char choice;
    int found;
    float Pay;

    // Opening file
    if (argc >= 2) {
        if ((BoatFile = fopen(argv[1], "r")) == NULL) {
            perror("Opening boat file");
            exit(EXIT_FAILURE);
        }
    } else {
        printf("Must provide file name as argument\n");
        exit(EXIT_FAILURE);
    }

    // Welcome
    printf("Welcome to the Boat Management System\n");
    printf("-------------------------------------\n");

    // Loading boats into  my BoatRecord and sorting
    LoadBoats(BoatRecord, argv[1], &totalBoats);
    qsort(BoatRecord, totalBoats, sizeof(BoatRecordType *), CompareBoats);

    do {
        // Print menu options and get user choice
        printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
        scanf(" %c", &choice);

        switch (tolower(choice)) {
            // Printing sorted
            case 'i':
                PrintInventory(BoatRecord, totalBoats);
                break;
            // Adding boat data
            case 'a': {
                char boatData[1000] = {0};  // Initialize to zeros
                printf("Please enter the boat data in CSV format                 : ");
                scanf(" %[^\n]", boatData);
                AddBoat(BoatRecord, &totalBoats, boatData);
                qsort(BoatRecord, totalBoats, sizeof(BoatRecordType *), CompareBoats);
                break;
            }
            // Removing boat by name
            case 'r':
                printf("Please enter the boat name                               : ");
                scanf(" %[^\n]", Name);
                RemoveBoat(BoatRecord, &totalBoats, Name);
                break;
            // Accepting Boat Payment
            case 'p':
                found = 0;
                printf("Please enter the boat name                               : ");
                scanf(" %[^\n]", Name);
                for (int i = 0; i < totalBoats; i++) {
                    if (strcasecmp(BoatRecord[i]->BoatName, Name) == 0) {
                       Pay = 0.0f;
                       printf("Please enter the amount to be paid: ");
                       scanf("%f", &Pay);
                       AcceptPayment(BoatRecord, i, Pay);
                       found = 1;
                       break;
                    }
                }
                if (!found)
                  printf("No boat with that name\n");
                break;

            case 'm':
              MonthlyUpdate(BoatRecord, totalBoats);
              break;

            case 'x':
                SaveBoats(BoatRecord, argv[1], totalBoats);
                printf("\nExiting the Boat Management System\n");
                break;

            default:
                printf("Invalid option %c\n", choice);
                break;
        }
    } while (tolower(choice) != 'x');

    // Close boat file
    if (fclose(BoatFile) == EOF) {
        perror("Closing boat file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < totalBoats; i++) {
        free(BoatRecord[i]);
    }

    return(EXIT_SUCCESS);
}
//-------------------------------------------------------------------------------------------------
// Function to allocate memory
void *Malloc(size_t Size) {
    void *ptr = malloc(Size);
    if (ptr == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}
//-------------------------------------------------------------------------------------------------
// Function to compare boats by name alphabetically
int CompareBoats(const void *A, const void *B) {
    BoatRecordType *boatA = *(BoatRecordType **)A;
    BoatRecordType *boatB = *(BoatRecordType **)B;
    return strcasecmp(boatA->BoatName, boatB->BoatName);
}
//-----------------------------------------------------------------------------
//----Convert a string to a place
PlaceType StringToPlaceType(char * PlaceString) {

    if (!strcasecmp(PlaceString,"slip")) {
        return(slip);
    }
    if (!strcasecmp(PlaceString,"land")) {
        return(land);
    }
    if (!strcasecmp(PlaceString,"trailor")) {
        return(trailor);
    }
    if (!strcasecmp(PlaceString,"storage")) {
        return(storage);
    }
    return(no_place);
}
//-----------------------------------------------------------------------------
//----Convert a place to a string
char *PlaceToString(PlaceType Place) {

    switch (Place) {
        case slip:
            return("slip");
        case land:
            return("land");
        case trailor:
            return("trailor");
        case storage:
            return("storage");
        case no_place:
            return("no_place");
        default:
            printf("How the faaark did I get here?\n");
            exit(EXIT_FAILURE);
            break;
    }
}
//-----------------------------------------------------------------------------
// Function to load boats
void LoadBoats(BoatRecordType *BoatRecord[MAX_BOATS], char *filename, int *totalBoats) {
    FILE *InputFile;
    PlaceString placeStr;
    *totalBoats = 0;
    if ((InputFile = fopen(filename, "r")) == NULL) {
        printf("Error opening file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    while (*totalBoats < MAX_BOATS) {
        // Temporary boat before allocating memory
        BoatRecordType tempBoat;

        // Read and validate initial input
        int input = fscanf(InputFile, "%[^,],%d,%[^,],", tempBoat.BoatName, &tempBoat.length, placeStr);
        if (input == EOF) {
            break;
        } else if (input != 3) {
            printf("Invalid data format\n");
            exit(EXIT_FAILURE);
        }

        tempBoat.Place = StringToPlaceType(placeStr);

        // Read additional data based on place type
        switch(tempBoat.Place) {
            case slip:
                fscanf(InputFile, "%d,", &tempBoat.ExtraInfo.SlipData);
                break;
            case land:
                fscanf(InputFile, "%c,", &tempBoat.ExtraInfo.LandData);
                break;
            case trailor:
                fscanf(InputFile, "%[^,],", tempBoat.ExtraInfo.TrailorData);
                break;
            case storage:
                fscanf(InputFile, "%d,", &tempBoat.ExtraInfo.StorageData);
                break;
            case no_place:
                printf("Invalid place type.\n");
                exit(EXIT_FAILURE);
        }

        fscanf(InputFile, "%f\n", &tempBoat.MoneyOwed);

        // Now allocate and store data if input successful
        BoatRecord[*totalBoats] = malloc(sizeof(BoatRecordType));
        if (BoatRecord[*totalBoats] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }

        // Copy temporary boat data to boat record
        *(BoatRecord[*totalBoats]) = tempBoat;
        (*totalBoats)++;
    }

    if (fclose(InputFile) == EOF) {
        perror("Error closing input file");
        exit(EXIT_FAILURE);
    }
}

//-----------------------------------------------------------------------------
// Function to add a boat
void AddBoat(BoatRecordType *BoatRecord[MAX_BOATS], int *totalBoats, char *boatData) {
    int i = *totalBoats;
    PlaceString placeStr;

    if (i >= MAX_BOATS) {
        printf("Cannot add boat - the marina is full.\n");
        return;
    }

    BoatRecord[i] = (BoatRecordType *)Malloc(sizeof(BoatRecordType));
    sscanf(boatData, "%[^,],%d,%[^,]", BoatRecord[i]->BoatName, &BoatRecord[i]->length, placeStr);

    BoatRecord[i]->Place = StringToPlaceType(placeStr);

    switch(BoatRecord[i]->Place) {
        case slip:
            sscanf(boatData, "%*[^,],%*[^,],%*[^,],%d", &BoatRecord[i]->ExtraInfo.SlipData);
            break;
        case land:
            sscanf(boatData, "%*[^,],%*[^,],%*[^,],%c", &BoatRecord[i]->ExtraInfo.LandData);
            break;
        case trailor:
            sscanf(boatData, "%*[^,],%*[^,],%*[^,],%[^,]", BoatRecord[i]->ExtraInfo.TrailorData);
            break;
        case storage:
            sscanf(boatData, "%*[^,],%*[^,],%*[^,],%d", &BoatRecord[i]->ExtraInfo.StorageData);
            break;
        default:
          printf("%s.\n", PlaceToString(BoatRecord[i]->Place));
            break;
    }

    sscanf(boatData, "%*[^,],%*[^,],%*[^,],%*[^,],%f", &BoatRecord[i]->MoneyOwed);

    (*totalBoats)++;

}
//-----------------------------------------------------------------------------
// Function to remove a boat
void RemoveBoat(BoatRecordType *BoatRecord[MAX_BOATS], int *totalBoats, char *boatName) {
    for (int i = 0; i < *totalBoats; i++) {
        if (strcasecmp(BoatRecord[i]->BoatName, boatName) == 0) {
            free(BoatRecord[i]);
            for (int j = i; j < *totalBoats - 1; j++) {
                BoatRecord[j] = BoatRecord[j + 1];
            }
            BoatRecord[*totalBoats - 1] = NULL;
            (*totalBoats)--;
            return;
        }
    }
    printf("No boat with that name\n");
}
//-----------------------------------------------------------------------------
// Function to accept a payment, including validation
void AcceptPayment(BoatRecordType *BoatRecord[MAX_BOATS], int index, float Pay) {

    if (Pay > BoatRecord[index]->MoneyOwed) {
                printf("That is more than the amount owed, $%.2f\n", BoatRecord[index]->MoneyOwed);
                return;
    }
    BoatRecord[index]->MoneyOwed -= Pay;

}
//-----------------------------------------------------------------------------
// Function to update monthly charges
void MonthlyUpdate(BoatRecordType *BoatRecord[MAX_BOATS], int totalBoats) {
    for (int i = 0; i < totalBoats; i++) {
        float monthlyCharge = 0.0f;

        switch (BoatRecord[i]->Place) {
            case slip:
                monthlyCharge = 12.50f * BoatRecord[i]->length;
                break;
            case land:
                monthlyCharge = 14.00f * BoatRecord[i]->length;
                break;
            case trailor:
                monthlyCharge = 25.00f * BoatRecord[i]->length;
                break;
            case storage:
                monthlyCharge = 11.20f * BoatRecord[i]->length;
                break;
            default:
                printf("%s - cannot update monthly charge.\n", PlaceToString(BoatRecord[i]->Place));
                break;
        }

        BoatRecord[i]->MoneyOwed += monthlyCharge;
    }
}
//-----------------------------------------------------------------------------
// Function to print boat inventory
void PrintInventory(BoatRecordType *BoatRecord[MAX_BOATS], int totalBoats) {
    for (int i = 0; i < totalBoats; i++) {
        BoatRecordType *Boat = BoatRecord[i];
        printf("%-20s %3d'  %-8s ", Boat->BoatName, Boat->length, PlaceToString(Boat->Place));

        switch (Boat->Place) {
            case slip:
              printf("%3s %d", "#", Boat->ExtraInfo.SlipData);
                break;
            case land:
                printf("%6c", Boat->ExtraInfo.LandData);
                break;
            case trailor:
                printf("%6s", Boat->ExtraInfo.TrailorData);
                break;
            case storage:
              printf("%3s %d", "#", Boat->ExtraInfo.StorageData);
                break;
            case no_place:
                break;
            default:
                break;
        }

        printf("   Owes $%7.2f\n", Boat->MoneyOwed);
    }
}
//-----------------------------------------------------------------------------
// Function to save to CSV
void SaveBoats(BoatRecordType *BoatRecord[MAX_BOATS], char *filename, int totalBoats) {
    FILE *File;

    if ((File = fopen(filename, "w")) == NULL) {
        printf("Error opening file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < totalBoats; i++) {
        BoatRecordType *boat = BoatRecord[i];
        fprintf(File, "%s,%d,%s,", boat->BoatName, boat->length, PlaceToString(boat->Place));

        switch (boat->Place) {
            case slip:
                fprintf(File, "%d,", boat->ExtraInfo.SlipData);
                break;
            case land:
                fprintf(File, "%c,", boat->ExtraInfo.LandData);
                break;
            case trailor:
                fprintf(File, "%s,", boat->ExtraInfo.TrailorData);
                break;
            case storage:
                fprintf(File, "%d,", boat->ExtraInfo.StorageData);
                break;
            default:
                fprintf(File, ",");
        }

        fprintf(File, "%.2f\n", boat->MoneyOwed);
    }

    if (fclose(File) == EOF) {
        perror("Closing file");
        exit(EXIT_FAILURE);
    }
}
