int8_t answer;
int onModulePin= 2;
char aux_string[30];
int flag = 0;
char number [20];
char realnumber[9];
char mynumber[9];
int a=0;
int b=0;
int c=0;
//Your phone number
char phone_number[]="012345678";  

char data[100];
int data_size;

char aux_str[30];
char aux;
int x = 0;
char N_S,W_E;

char url[] = "pruebas.libelium.com";
char frame[200];

char latitude[15];
char longitude[15];
char altitude[6];
char date[16];
char time[7];
char satellites[3];
char speedOTG[10];
char course[10];

void setup(){
    mynumber[0]='0';
    mynumber[1]='1';
    mynumber[2]='2';
    mynumber[3]='3';
    mynumber[4]='4';
    mynumber[5]='5';
    mynumber[6]='6';
    mynumber[7]='7';
    mynumber[8]='8';
    

    pinMode(onModulePin, OUTPUT);
    Serial.begin(115200);    
    
    power_on();
    power_onGPS();
    power_onSMS();
    
    delay(5000);
    
    sendATcommand("AT+CPIN=****", "OK", 2000);  
  
    delay(3000);  

    while( (sendATcommand("AT+CREG?", "+CREG: 0,1", 1000) || sendATcommand("AT+CREG?", "+CREG: 0,5", 1000)) == 0 );


    sendATcommand("AT+CLIP=1", "OK", 1000); 
    
    while ( start_GPS() == 0);

    while (sendATcommand("AT+CREG?", "+CREG: 0,1", 2000) == 0);

    // sets APN , user name and password
    sendATcommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"", "OK", 2000);
    sendATcommand("AT+SAPBR=3,1,\"APN\",\"*******\"", "OK", 2000);
    sendATcommand("AT+SAPBR=3,1,\"USER\",\"*******\"", "OK", 2000);
    sendATcommand("AT+SAPBR=3,1,\"PWD\",\"*******\"", "OK", 2000);

    // gets the GPRS bearer
    while (sendATcommand("AT+SAPBR=1,1", "OK", 20000) == 0)
    {
        delay(5000);
    }
    
    delay(1000);
    while(Serial.available() != 0)
    {
      Serial.read();  
    }
}
void loop(){
    
    answer = sendATcommand("", "+CLIP", 1000);
    //Detect incomming call
    if (answer == 1)
    {
        Serial.println("Incoming call");          
          
          if ( flag == 0){
            
              for (int i=0; i<19; i++){
               // read the incoming byte:
               while (Serial.available() == 0)
               {                 
                 delay (50);
               }
               //Stores phone number
               number[i] = Serial.read();  
               
              }
              Serial.flush();
              flag = 1; 
          }          
          //Stores phone calling number
          for (int i=0; i<=14; i++){
            if(number[i]== '"'){
              i++;
              realnumber[0]=number[i];
              i++;
              realnumber[1]=number[i];
              i++;
              realnumber[2]=number[i];
              i++;
              realnumber[3]=number[i];
              i++;
              realnumber[4]=number[i];
              i++;
              realnumber[5]=number[i];
              i++;
              realnumber[6]=number[i];
              i++;
              realnumber[7]=number[i];
              i++;
              realnumber[8]=number[i];
              break;
            }            
    }
    //Check phone number
    for (int i=0;i<9;i++){ 
      if (realnumber[i] == mynumber[i]){
          a++;       
          if( a==9){  
             Serial.println("Correct number");
             sendATcommand("ATH", "OK", 1000);
               if(b==1){
                  b=0;
                }else{                  
                  b=1;
                  c=1;
                    }
                    break;
                  }
                }else{             
                  Serial.println("Wrong number");
                  break;
                 }
              }          
     a=0;
     answer=0;
     flag = 0;     
}
//Send SMS once and position to HTTP                   
if (b==1){
     get_GPS();
     send_HTTP();
     delay(500);                                       
    if (c==1){
      sendSMS();
      delay(100);
      c=0;                   
      }               
     }
}

void power_on(){

    uint8_t answer=0;

    digitalWrite(onModulePin,HIGH);
    delay(3000);
    digitalWrite(onModulePin,LOW);

    while(answer == 0){     // Send AT every two seconds and wait for the answer
        answer = sendATcommand("AT", "OK", 2000);    
    }
}

int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout){

    uint8_t x=0,  answer=0;
    char response[100];
    unsigned long previous;

    memset(response, '\0', 100);    // Initialice the string
    
    delay(100);
    
    while( Serial.available() > 0) Serial.read();    // Clean the input buffer
    
    if (ATcommand[0] != '\0')
    {
        Serial.println(ATcommand);    // Send the AT command 
    }


    x = 0;
    previous = millis();

    // this loop waits for the answer
    do{
        if(Serial.available() != 0){    // if there are data in the UART input buffer, reads it and checks for the asnwer
            response[x] = Serial.read();
            //Serial.print(response[x]);
            x++;
            if (strstr(response, expected_answer) != NULL)    // check if the desired answer (OK) is in the response of the module
            {
                answer = 1;
            }
        }
    }while((answer == 0) && ((millis() - previous) < timeout));    // Waits for the asnwer with time out

    return answer;
}

void power_onGPS(){

    uint8_t answer=0;

    // checks if the module is started
    answer = sendATcommand("AT", "OK", 2000);
    if (answer == 0)
    {
        // power on pulse
        digitalWrite(onModulePin,HIGH);
        delay(3000);
        digitalWrite(onModulePin,LOW);

        // waits for an answer from the module
        while(answer == 0){  
            // Send AT every two seconds and wait for the answer   
            answer = sendATcommand("AT", "OK", 2000);    
        }
    }

}

int8_t start_GPS(){

    unsigned long previous;

    previous = millis();
    // starts the GPS
    sendATcommand("AT+CGPSPWR=1", "OK", 2000);
    sendATcommand("AT+CGPSRST=0", "OK", 2000);

    // waits for fix GPS
    while(( (sendATcommand("AT+CGPSSTATUS?", "2D Fix", 5000) || 
        sendATcommand("AT+CGPSSTATUS?", "3D Fix", 5000)) == 0 ) && 
        ((millis() - previous) < 90000));

    if ((millis() - previous) < 90000)
    {
        return 1;
    }
    else
    {
        return 0;    
    }
}

int8_t get_GPS(){

    int8_t counter, answer;
    long previous;

    // First get the NMEA string
    // Clean the input buffer
    while( Serial.available() > 0) Serial.read(); 
    // request Basic string
    sendATcommand("AT+CGPSINF=0", "AT+CGPSINF=0\r\n\r\n", 2000);

    counter = 0;
    answer = 0;
    memset(frame, '\0', 100);    // Initialize the string
    previous = millis();
    // this loop waits for the NMEA string
    do{

        if(Serial.available() != 0){    
            frame[counter] = Serial.read();
            counter++;
            // check if the desired answer is in the response of the module
            if (strstr(frame, "OK") != NULL)    
            {
                answer = 1;
            }
        }
        // Waits for the asnwer with time out
    }
    while((answer == 0) && ((millis() - previous) < 2000));  

    frame[counter-3] = '\0'; 
    
    // Parses the string 
    strtok(frame, ",");
    strcpy(longitude,strtok(NULL, ",")); // Gets longitude
    strcpy(latitude,strtok(NULL, ",")); // Gets latitude
    strcpy(altitude,strtok(NULL, ".")); // Gets altitude 
    strtok(NULL, ",");    
    strcpy(date,strtok(NULL, ".")); // Gets date
    strtok(NULL, ",");
    strtok(NULL, ",");  
    strcpy(satellites,strtok(NULL, ",")); // Gets satellites
    strcpy(speedOTG,strtok(NULL, ",")); // Gets speed over ground. Unit is knots.
    strcpy(course,strtok(NULL, "\r")); // Gets course

    convert2Degrees(latitude);
    convert2Degrees(longitude);
    
    return answer;
}

/* convert2Degrees ( input ) - performs the conversion from input 
 * parameters in  DD°MM.mmm’ notation to DD.dddddd° notation. 
 * 
 * Sign '+' is set for positive latitudes/longitudes (North, East)
 * Sign '-' is set for negative latitudes/longitudes (South, West)
 *  
 */
int8_t convert2Degrees(char* input){

    float deg;
    float minutes;
    boolean neg = false;    

    //auxiliar variable
    char aux[10];

    if (input[0] == '-')
    {
        neg = true;
        strcpy(aux, strtok(input+1, "."));

    }
    else
    {
        strcpy(aux, strtok(input, "."));
    }

    // convert string to integer and add it to final float variable
    deg = atof(aux);

    strcpy(aux, strtok(NULL, '\0'));
    minutes=atof(aux);
    minutes/=1000000;
    if (deg < 100)
    {
        minutes += deg;
        deg = 0;
    }
    else
    {
        minutes += int(deg) % 100;
        deg = int(deg) / 100;    
    }

    // add minutes to degrees 
    deg=deg+minutes/60;


    if (neg == true)
    {
        deg*=-1.0;
    }

    neg = false;

    if( deg < 0 ){
        neg = true;
        deg*=-1;
    }
    
    float numberFloat=deg; 
    int intPart[10];
    int digit; 
    long newNumber=(long)numberFloat;  
    int size=0;
    
    while(1){
        size=size+1;
        digit=newNumber%10;
        newNumber=newNumber/10;
        intPart[size-1]=digit; 
        if (newNumber==0){
            break;
        }
    }
   
    int index=0;
    if( neg ){
        index++;
        input[0]='-';
    }
    for (int i=size-1; i >= 0; i--)
    {
        input[index]=intPart[i]+'0'; 
        index++;
    }

    input[index]='.';
    index++;

    numberFloat=(numberFloat-(int)numberFloat);
    for (int i=1; i<=6 ; i++)
    {
        numberFloat=numberFloat*10;
        digit= (long)numberFloat;          
        numberFloat=numberFloat-digit;
        input[index]=char(digit)+48;
        index++;
    }
    input[index]='\0';


}

void send_HTTP(){
    
    uint8_t answer=0;
    // Initializes HTTP service
    answer = sendATcommand("AT+HTTPINIT", "OK", 10000);
    if (answer == 1)
    {
        // Sets CID parameter
        answer = sendATcommand("AT+HTTPPARA=\"CID\",1", "OK", 5000);
        if (answer == 1)
        {
            // Sets url 
            sprintf(aux_str, "AT+HTTPPARA=\"URL\",\"http://%s/demo_sim908.php?", url);
            Serial.print(aux_str);
            sprintf(frame, "visor=false&latitude=%s&longitude=%s&altitude=%s&time=%s&satellites=%s&speedOTG=%s&course=%s",
            latitude, longitude, altitude, date, satellites, speedOTG, course);
            Serial.print(frame);
            answer = sendATcommand("\"", "OK", 5000);
            if (answer == 1)
            {
                // Starts GET action
                answer = sendATcommand("AT+HTTPACTION=0", "+HTTPACTION:0,200", 30000);
                if (answer == 1)
                {

                    Serial.println(F("Done!"));
                }
                else
                {
                    Serial.println(F("Error getting url"));
                }

            }
            else
            {
                Serial.println(F("Error setting the url"));
            }
        }
        else
        {
            Serial.println(F("Error setting the CID"));
        }    
    }
    else
    {
        Serial.println(F("Error initializating"));
    }

    sendATcommand("AT+HTTPTERM", "OK", 5000);
    
}

void power_onSMS(){

    uint8_t answer=0;
    
    // checks if the module is started
    answer = sendATcommand("AT", "OK", 2000);
    if (answer == 0)
    {
        // power on pulse
        digitalWrite(onModulePin,HIGH);
        delay(3000);
        digitalWrite(onModulePin,LOW);
    
        // waits for an answer from the module
        while(answer == 0){     // Send AT every two seconds and wait for the answer
            answer = sendATcommand("AT", "OK", 2000);    
        }
    }
    
}

void sendSMS(){
  sendATcommand("AT+CPIN=****", "OK", 2000);
    
    delay(3000);
    
    Serial.println("Connecting to the network...");

    while( (sendATcommand("AT+CREG?", "+CREG: 0,1", 500) || 
            sendATcommand("AT+CREG?", "+CREG: 0,5", 500)) == 0 );

    Serial.print("Setting SMS mode...");
    sendATcommand("AT+CMGF=1", "OK", 1000);    // sets the SMS mode to text
    Serial.println("Sending SMS");
    
    sprintf(aux_string,"AT+CMGS=\"%s\"", phone_number);
    answer = sendATcommand(aux_string, ">", 2000);    // send the SMS number
    if (answer == 1)
    {
        Serial.print("Help me! I've been stolen. Find me in:");
        Serial.print("Latitude: ");
              int i = 0;
              while(latitude[i]!=0){
              Serial.print(latitude[i]);
              i++;
              }
              Serial.print(" / Longitude: ");
              i = 0;
              while(longitude[i]!=0){
              Serial.print(longitude[i]);
              i++;
              }
        Serial.write(0x1A);
        answer = sendATcommand("", "OK", 20000);
        if (answer == 1)
        {
            Serial.print("Sent ");    
        }
        else
        {
            Serial.print("error ");
        }
    }
    else
    {
        Serial.print("error ");
        Serial.println(answer, DEC);
    }

}