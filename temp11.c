// dht11.c
// compile line
// gcc -Wall -std=gnu99 temp11.c -o temp11 -l wiringPi

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#define MAX_TIME 85
#define ATTEMPTS 5
#define DHT11PIN 7
int dht11_val[5]={0,0,0,0,0};

int dht11_read_val()
{
    float humid=0.0;
    float tempC=0.0;
    float tempF=0.0;
    uint8_t lststate=HIGH;
    uint8_t counter=0;
    uint8_t j=0,i;
    int addr, nemail;
    char out_string[200];
    time_t current_time;
    current_time=time(NULL);

// here are the text equivalents for various phone companies if you prefer
// text messages to plain email
//
// AT&T: [number]@txt.att.net
// Sprint: [number]@messaging.sprintpcs.com or [number]@pm.sprint.com
// T-Mobile: [number]@tmomail.net
// Verizon: [number]@vtext.com
// Boost Mobile: [number]@myboostmobile.com
// Cricket: [number]@sms.mycricket.com
// Metro PCS: [number]@mymetropcs.com
// Tracfone: [number]@mmst5.tracfone.com
// U.S. Cellular: [number]@email.uscc.net
// Virgin Mobile: [number]@vmobl.com 

// customize the next four lines for your needs, temperatures in Farenheit
    float alarmhi=80.0;
    float alarmlo=40.0;
    const char *email[2]={"phonenumber@vtext.com","youremail@gmail.com"};
    const char *loc="Your RV";

    nemail = sizeof(email)/sizeof(email[0]);

    for(i=0;i<5;i++)
         dht11_val[i]=0;

    pinMode(DHT11PIN,OUTPUT);
    digitalWrite(DHT11PIN,LOW);
    delay(18);
    digitalWrite(DHT11PIN,HIGH);
    delayMicroseconds(40);
    pinMode(DHT11PIN,INPUT);

    for(i=0;i<MAX_TIME;i++)
    {
        counter=0;
        while(digitalRead(DHT11PIN)==lststate){
            counter++;
            delayMicroseconds(1);
            if(counter==255)
                break;
        }
        lststate=digitalRead(DHT11PIN);
        if(counter==255)
             break;
// top 3 transistions are ignored
        if((i>=4)&&(i%2==0)){
            dht11_val[j/8]<<=1;
            if(counter>16)
                dht11_val[j/8]|=1;
            j++;
        }
    }

// verify checksum and print the verified data
// dht11 has only one digit precision
    if((j>=40)&&(dht11_val[4]==((dht11_val[0]+dht11_val[1]+dht11_val[2]+dht11_val[3])& 0xFF)))
    {
        humid=1.*dht11_val[0];
        tempC=1.*dht11_val[2];
        tempF=(9.*tempC/5.)+32.;

// uncomment the next line to test email system
//       tempF=100,;

        printf("%5.1f C\t%5.1f F\t%5.1f\t%s",tempC,tempF,humid,ctime(&current_time));

// send e-mail if over or under temperature limits
        if(tempF > alarmhi) {
            for (addr=0; addr<nemail; addr++) {
                sprintf(out_string,
                "echo '%s high temp %5.1f F alarm at %s' | "
                "heirloom-mailx -s '%s high temp %5.1fF alarm' %s",
                loc, tempF, ctime(&current_time), loc, tempF, email[addr]);
//              printf("%s\n",out_string);
                system(out_string);
            }
        } else if (tempF < alarmlo) {
            for (addr=0; addr<nemail; addr++) {
                sprintf(out_string,
                "echo '%s low temp %5.1f F alarm at %s' | "
                "heirloom-mailx -s '%s low temp %5.1fF alarm' %s",
                loc, tempF, ctime(&current_time), loc, tempF, email[addr]);
//              printf("%s\n",out_string);
                system(out_string);
            }
        }
        return 1;
    }
    else
        return 0;
}

int main(void)
{
    int attempts=ATTEMPTS;
    if(wiringPiSetup()==-1)
        exit(1);
    while(attempts)
    {
        int success = dht11_read_val();
        if (success) {
            break;
        }
        attempts--;
        delay(500);
    }
    return 0;
}
