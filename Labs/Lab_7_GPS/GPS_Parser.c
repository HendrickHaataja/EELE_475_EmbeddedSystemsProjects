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
void convertTime(char time[3], char c1, char c2);

FILE *LCD_init();
void LCD_terminate(FILE *lcd);
void LCD_print(char * string, FILE *lcd);
//   for(int i=0; i<400; i++){
//    printf("%c", data_set[i]);
//    }

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

/*
//this main function is for when the code is running on the 
//NIOS processor on the FPGA.
int main(void)
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
*/

//This main function is for the homework assignment, and testing purposes.
int main(void){
  FILE *gps_data;
  char c;
  int index = 0;
  
  gps_data = fopen("GPS_characters.txt", "r");
  while((c = fgetc(gps_data)) != EOF){
   index = read_char(c, index);
  }
  return 0;
} 

//This function mimics our LCD_print function from lab 6,
//except it uses printf and does not do error checking.
void print(char * string){
//  LCD_print(string,lcd_global);
  printf("%s\n", string);
}
/*
*/
int read_char(char c, int index){
  if(c == '$'){
    data_parser();
    index = 0;
  }
  data_set[index] = c;
 // printf("data_set[%i] is %c",index,c);
  index += 1;
  return index;
}

/*data_parser looks if data set is one of the two we are interested in, 
 *then checks the checksum. If checksum if valid, it calls the parsing
 *function, and data will be printed out.
*/
void data_parser(void){
	//printf("in data parser\n");
	if(data_set[1]=='G' && data_set[2]=='P' && data_set[3]=='G' && data_set[5]=='A'){
    if(data_set[4]=='G'){
      //printf("found gga\n");
      if(checksum()){
        parse_GGA();
      }
    }else if(data_set[4]=='S'){
      //printf("found gsa\n");
      if(checksum()){
        parse_GSA();
      }
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
      	//printf("I found a newline\n");
	end = 1;
       //printf("got to end\n");
     }else if(data_set[dataSetIndex] == '*'){
      //stop when get to checksum, because already 
      //have looked at checksum to see if dataset is valid.
      //checksum time
      //printf("got to checksum\n");
      end = 1;
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
	char UTC_posTime[] = "Time: XX:XX. (UTC)";
	char local_time[] = "Time: XX:XX XX. (local)";	
	UTC_posTime[6] = field[0];
	UTC_posTime[7] = field[1];
	UTC_posTime[9] = field[2];
	UTC_posTime[10] = field[3];
	char time[3];
        convertTime(time, field[0], field[1]); 
	local_time[6] = time[0];
	local_time[7] = time[1];
	local_time[9] = field[2];
	local_time[10] = field[3];
	if(time[2] == '1'){
	  local_time[12] = 'P';
	}else if(time[2] == '0'){
	  local_time[12] = 'A';
	}
	local_time[13] = 'M';
	print(local_time);
	print(UTC_posTime);
        break;
      case(2): ;
	//Latitude
	char latitude[] = "Lat: XX deg XX min X";
	latitude[5] = field[0];
	latitude[6] = field[1];
	latitude[12] = field[2];
	latitude[13] = field[3];	
	latitude[19] = data_set[dataSetIndex + 1]; //gets N or S.
	print(latitude);
       // printf("Latitude: %c%c degrees %c%c%c%c%c%c%c%c min ",field[0],field[1],field[2],field[3],field[4],field[5],field[6],field[7],field[8],field[9]);
        break;
      case(3):
	//take care of this in latitude now.
	//North or south?
//	if(field[curFieldIndex-1] == 'N') printf("north\n");
//	if(field[curFieldIndex] == 'S') printf("south\n");
        break;
      case(4): ;
	//Longitude
	char longitude[] = "Long: XXX deg XX min X";
	longitude[6] = field[0];
	longitude[7] = field[1];
	longitude[8] = field[2];
	longitude[14] = field[3];
	longitude[15] = field[4];
	longitude[21] = data_set[dataSetIndex + 1];
	print(longitude);
//	printf("Longitude: %c%c%c degrees %c%c%c%c%c%c%c%c min ",field[0],field[1],field[2],field[3],field[4],field[5],field[6],field[7],field[8],field[9],field[10]);
        break;
      case(5):
	//take care of this in longitude now
	//east or west?
	//if(field[curFieldIndex-1] == 'W') printf("West\n");
	//if(field[curFieldIndex-1] == 'E') printf("East\n");
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
      	//printf("I found a newline\n");
	end = 1;
       //printf("got to end\n");
     }else if(data_set[dataSetIndex] == '*'){
      //stop when get to checksum, because already 
      //have looked at checksum to see if dataset is valid.
      //checksum time
      // printf("got to checksum\n");
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

//Function returns a 1 if dataset is valid, a 0 if it is not.
int checksum(){
//code to get the checksum
  int j = 0;
  char rightNibble;
  char leftNibble;
  while(data_set[j] != '*'){
    j++;
  }
  leftNibble = data_set[j+1];
  rightNibble = data_set[j+2];
//  printf("left nibble is %c\n", leftNibble);
//  printf("right nibble is %c\n", rightNibble);

//code to calculate the checksum
  char right4bits = 0x0F;
  char left4bits = 0xF0;
  int i = 2;
  char cur = data_set[1];
 
  while(data_set[i] != '*'){
    cur ^= data_set[i];
    i++;
  }  

  left4bits &= cur;
  right4bits &= cur;

  left4bits = left4bits >> 4;  
//  printf("last 4 bits are %i\n",left4bits);
//  printf("first 4 bits are %i\n",right4bits);
  if(0 <= (int)right4bits && 10 > (int)right4bits){
    right4bits = '0' + (int)right4bits; 
  }
  if(10 <= (int)right4bits && 16 > (int)right4bits){
    right4bits = 'A' + (int)right4bits - 10;
  }
  if(0 <= (int)left4bits && 160 > (int)left4bits){
   left4bits = '0' + (int)left4bits;
//   printf("%i\n", (int)left4bits);
  }
  if(160 <= (int)left4bits && 240 >= (int)left4bits){
    left4bits = 'A' + left4bits - 10;
  }
//  printf("Left 4 bits in hex are %c\n",left4bits);
//  printf("Right 4 bits in hex are %c\n",right4bits);
  if(left4bits == leftNibble && right4bits == rightNibble){
    return 1;
  }else{
    return 0;
  }
}

void convertTime(char time[3], char c1, char c2){
  int hours = 0;
  hours = (c1 - '0')*10 + (c2 - '0');
  hours -= 7;  
  int isPM = 0;
  if(hours >= 12){
    //PM 
    isPM = 1;//if last character of time is 1, PM
    if(hours >= 13){
      hours -= 12;
    }
  }else{
   if(hours = 0){
     hours = 12;
   }
    //AM 
    isPM = 0;
  }
  time[0] = '0' + (hours / 10);
  time[1] = '0' + (hours % 10);
  time[2] = '0' + isPM;
  //printf("hours are %c%c isPM is %c\n",time[0], time[1], time[2]);
 // return time;
}
