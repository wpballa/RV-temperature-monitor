// tempRV.c
// compile line
// gcc -Wall -std=gnu99 tempRV.c -o tempRV -l wiringPi

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#define MAX_TIME 85
#define ATTEMPTS 5
#define DHTPIN 7
int dht_val[5]={0,0,0,0,0};

int dht_read_val()
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

// customize the next five lines for your needs, temperatures in Farenheit, adjust dimension in
// the email vector as needed for your list
    int dht = 22;       // set this to 11 or 22 depending on your sensor type
    float alarmhi=80.0;
    float alarmlo=40.0;
    const char *email[2]={"phonenumber@vtext.com","youremail@gmail.com"};
    const char *loc="Your RV";

    nemail = sizeof(email)/sizeof(email[0]);

    for(i=0;i<5;i++)
         dht_val[i]=0;

    pinMode(DHTPIN,OUTPUT);
    digitalWrite(DHTPIN,LOW);
    delay(18);
    digitalWrite(DHTPIN,HIGH);
    delayMicroseconds(40);
    pinMode(DHTPIN,INPUT);

    for(i=0;i<MAX_TIME;i++)
    {
        counter=0;
        while(digitalRead(DHTPIN)==lststate){
            counter++;
            delayMicroseconds(1);
            if(counter==255)
                break;
        }
        lststate=digitalRead(DHTPIN);
        if(counter==255)
             break;
// top 3 transistions are ignored
        if((i>=4)&&(i%2==0)){
            dht_val[j/8]<<=1;
            if(counter>16)
                dht_val[j/8]|=1;
            j++;
        }
    }

// verify checksum and print the verified data
// dht has only one digit precision while dht22 has much more precision
    if((j>=40)&&(dht_val[4]==((dht_val[0]+dht_val[1]+dht_val[2]+dht_val[3])& 0xFF)))
    {
        if (dht==11)
        {
            humid=1.*dht_val[0];
            tempC=1.*dht_val[2];
        }
        else if (dht==22)
        {
            humid=(256.*dht_val[0]+dht_val[1])/10.;
            tempC=(256.*dht_val[2]+dht_val[3])/10.;
        }
        else
        {
            printf(" Illegal sensor type selected, only 11 or 22 are acceptable\n");
        }

        tempF=(9.*tempC/5.)+32.;

// uncomment the next line to test email system
//       tempF=100,;

// this line prints to the screen or log file
        printf("%5.1f C\t%5.1f F\t%5.1f\t%s",tempC,tempF,humid,ctime(&current_time));

// send e-mail if over or under temperature limits to the list of email/text recipients
        if(tempF > alarmhi) {
            for (addr=0; addr<nemail; addr++) {
                sprintf(out_string,
                "echo '%s high temp %5.1f F alarm at %s' | "
                "heirloom-mailx -s '%s high temp %5.1fF alarm' %s",
                loc, tempF, ctime(&current_time), loc, tempF, email[addr]);
                system(out_string);
            }
        } else if (tempF < alarmlo) {
            for (addr=0; addr<nemail; addr++) {
                sprintf(out_string,
                "echo '%s low temp %5.1f F alarm at %s' | "
                "heirloom-mailx -s '%s low temp %5.1fF alarm' %s",
                loc, tempF, ctime(&current_time), loc, tempF, email[addr]);
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
        int success = dht_read_val();
        if (success) {
            break;
        }
        attempts--;
        delay(500);
    }
    return 0;
}
