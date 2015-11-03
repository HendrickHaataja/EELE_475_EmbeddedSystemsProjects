#include "gps.h"

#define SW_IDX 3
#define SWITCHES (volatile int *) 0x01001030
/* Declare one global variable to capture the output of the buttons (SW0-SW3),
 * when they are pressed.
 */

char data_set[200];
FILE *lcd_global;

//Function prototype declarations
int read_char(char c,int index);
void data_parser(void);
void parse_GGA(void);
void parse_GSA(void);
int checksum(void);


FILE *LCD_init();
void LCD_terminate(FILE *lcd);
void LCD_print(char * string, FILE *lcd);

FILE *LCD_init(){
  FILE *lcd;
  lcd = fopen("/dev/lcd_display", "w");
  return lcd;
}

void LCD_terminate(FILE *lcd){
  if(lcd){
	  fclose(lcd);
  }
}

/********************************************************
LCD_print takes a char pointer as an input.
The char pointer must be terminated in a null character.
The function then declares a file pointer that points to
the LCD, and uses fprintf to write the input string
pointed to by the input char pointer.
********************************************************/
void LCD_print(char * string, FILE * lcd)
{

  if(!string)
		return;

  if(lcd)
	  fprintf(lcd, "%s\n", string);
  return;
}


int main()
{
	lcd_global = LCD_init();
	char c;
	int index = 0;
	while(1){
		c=getchar();
		index = read_char(c,index);
		//printf("%c",c);
	}
	LCD_terminate(lcd_global);
	return 0;
}


#if 0
int main()
{
  int switch_vals = 0;
  int last_switch = 0;
  int current_switch = 0;


  while(1)
  {
	 int i;
	 switch_vals = (*SWITCHES);
	 switch_vals &= 0xF;

	 int numSetBits = 0;
	 for(i = 1;  i <= 0x8; i <<= 1)
	 {
		 if(i & switch_vals)
			 numSetBits++;
	 }
	 if(switch_vals & current_switch || numSetBits != 1)
		 continue;

    printf("switch_vals & current_switch => %d & %d = %d\n", switch_vals, current_switch, switch_vals & current_switch);
    printf("NumSetBits = %d\n", numSetBits);

	last_switch = current_switch;
	current_switch = switch_vals;
	printf("last_swtich = %d\n", last_switch);

	LCD_print_switch_status(current_switch, last_switch);
  }

  return 0;
}
#endif


//This function mimics our LCD_print function from lab 6,
//except it uses printf and does not do error checking.
void print(char * string){
  LCD_print(string,lcd_global);
  printf("%s\n", string);
}
/*
int main(void){
  FILE *gps_data;
  char c;
  gps_data = fopen("GPS_characters.txt", "r");
  while((c = fgetc(gps_data)) != EOF){
   read_char(c);
  }
  return 0;
}*/

/*
*/
int read_char(char c, int index){
  if(c == '$'){
//   for(int i=0; i<400; i++){
//    printf("%c", data_set[i]);
//    }

    data_parser();
    index = 0;
  }
  data_set[index] = c;
 // printf("data_set[%i] is %c",index,c);
  index += 1;
  return index;
}

void data_parser(void){
	//printf("in data parser\n");
	if(data_set[1]=='G' && data_set[2]=='P' && data_set[3]=='G' && data_set[5]=='A'){
    if(data_set[4]=='G'){
    	printf("found gga\n");
      parse_GGA();
    }else if(data_set[4]=='S'){
    	printf("found gsa\n");
      parse_GSA();
    }
  }
}

void parse_GGA(void){
  int i;
  int end = 0;
  int curFieldIndex = 0;
  int dataSetIndex = 0;
  int whichField = 0;
  char field[20];

  while(!end){
    if(data_set[dataSetIndex] == '\r' || data_set[dataSetIndex]=='\n'){
      	printf("I found a newline\n");
	end = 1;
       printf("got to end\n");
     }else if(data_set[dataSetIndex] == '*'){
      //checksum time
      printf("got to checksum\n");
      end = 1;//need to fix this
      break;
    }

    while(data_set[dataSetIndex] != ','){
      field[curFieldIndex] = data_set[dataSetIndex];
      dataSetIndex++;
      curFieldIndex++;
    }
    //In lab will switch print LCD function in for printf.
    switch(whichField){
      case(0):
	//Start of data set, do nothing
        break;
      case(1): ;
        //UTC Positional Time
	//declare a string with desired length, replace X's with desired values, call print function.
	char UTC_posTime[] = "UTC positional time: XXh XXmin XXXXXsec.\n";
	UTC_posTime[21] = field[0];
	UTC_posTime[22] = field[1];
	UTC_posTime[25] = field[2];
	UTC_posTime[26] = field[3];
	for(i=31; i<36; i++){
	  UTC_posTime[i] = field[i-27];
	}
	print(UTC_posTime);
//	printf("UTC positional time: %c%ch %c%cmin %c%c%c%c%csec\n",field[0],field[1],field[2],field[3],field[4],field[5],field[6],field[7],field[8]);
        break;
      case(2): ;
	//Latitude
	char latitude[] = "Latitude: XX degrees XXXXXXXX min, X";
	latitude[10] = field[0];
	latitude[11] = field[1];
	for(i=2; i<10; i++){
	  latitude[i+19] = field[i];
	}
	latitude[35] = data_set[dataSetIndex + 1];
	print(latitude);
        printf("Latitude: %c%c degrees %c%c%c%c%c%c%c%c min ",field[0],field[1],field[2],field[3],field[4],field[5],field[6],field[7],field[8],field[9]);
        break;
      case(3):
	//North or south?
	if(field[curFieldIndex-1] == 'N') printf("north\n");
	if(field[curFieldIndex] == 'S') printf("south\n");
        break;
      case(4): ;
	//Longitude
	char longitude[] = "Longitude: XXX degrees XXXXXXXX min, X";
	longitude[11] = field[0];
	longitude[12] = field[1];
	longitude[13] = field[2];
	for(i=3; i<11; i++){
	  longitude[i+20] = field[i];
	}
	longitude[37] = data_set[dataSetIndex + 1];
	print(longitude);
//	printf("Longitude: %c%c%c degrees %c%c%c%c%c%c%c%c min ",field[0],field[1],field[2],field[3],field[4],field[5],field[6],field[7],field[8],field[9],field[10]);
        break;
      case(5):
	//east or west?
	if(field[curFieldIndex-1] == 'W') printf("West\n");
	if(field[curFieldIndex-1] == 'E') printf("East\n");
        break;
      case(9): ;
	//This is kind of broken because there are a varying number of characters in field according to handout.
        char elevationStr[] = "Elevation: XXXXXX X.";
	for(i=11; i<17; i++){     elevationStr[i] = field[i-11];	}
	elevationStr[18] = data_set[dataSetIndex+1];
	print(elevationStr);
	break;
      default:
        break;
    }
    dataSetIndex++;
    curFieldIndex=0;
    whichField++;
  }
}

void parse_GSA(void){
  int end = 0;
  int dataSetIndex = 0;
  int whichField = 0;
  int id_index = 0;
  static  char satelliteIDs[30];

  while(!end){
    if(data_set[dataSetIndex] == '\r' || data_set[dataSetIndex]=='\n'){
      	printf("I found a newline\n");
	end = 1;
       printf("got to end\n");
     }else if(data_set[dataSetIndex] == '*'){
      //checksum time
      printf("got to checksum\n");
      end = 1;//need to fix this
      break;
    }
    if(whichField >=3 && whichField <= 10){
     // int i;
     // for(i=0; i<4; i++){
        satelliteIDs[id_index] = data_set[dataSetIndex];
     // }
      id_index++;
    }

    if(data_set[dataSetIndex] == ','){
      whichField++;
    }
     dataSetIndex++;
  }
  print("The satellite ID numbers are: ");
  print(satelliteIDs);
}

int checksum(void){
  int i = 2;
  char cur = data_set[1];
  while(data_set[i] != '*'){
    cur ^= data_set[i];
    i++;
  }
  return (int)cur;
}
