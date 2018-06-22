#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>

#define LED_R   3
#define LED_Y   2
#define LED_G   0

#define SW_R    6
#define SW_Y    5
#define SW_G    4
#define SW_W    25

void init( void );
void off( void );
void blink( void );
int checkAns( void );
void fail( void );
void viewRound( void );
void lights( void );
void redLight( void );
void yellowLight( void );
void greenLight( void );
sem_t semR, semY, semG, semW;

int nRoundAns[5] = {0,};
int userAns[5] = {0,};
int nRound = 1;
int nLight = 0;

int main(void)
{
	sem_init( &semR, 0, 1 );
	sem_init( &semY, 0, 1 );
	sem_init( &semG, 0, 1 );
	sem_init( &semW, 0, 1 );
	sem_wait( &semR );
	sem_wait( &semY );
	sem_wait( &semG );

	pthread_t   tR, tY, tG, tW;

	init();

	srand( time( NULL ) );
	for( nLight = 0 ; nLight < 5 ; nLight++ ){
		nRoundAns[nLight]=( rand()%3 ) + 1;
	}

	nLight = 0;
	sleep( 1 );

	viewRound();

	pthread_create( &tR, NULL, (void *)&redLight, NULL );
	pthread_create( &tY, NULL, (void *)&yellowLight, NULL );
	pthread_create( &tG, NULL, (void *)&greenLight, NULL );
	pthread_create( &tW, NULL, (void *)&lights, NULL );

	while( nRound < 6 )
	{
		if( digitalRead( SW_W ) == 0 )
		{
			if( checkAns() == 1 )
			{
				nRound++;
				if( nRound == 6 )
				{
					break;
				}
			}
			else
			{
				exit(1);
			}
						
			delay( 350 );
			viewRound();

			for( nLight = 0 ; nLight < 5 ; nLight++ ){
				userAns[nLight] = 0;
			}

			nLight = 0;
			delay( 250 );
		}
	}

	sem_destroy( &semR );
	sem_destroy( &semY );
	sem_destroy( &semG );
	sem_destroy( &semW );
	return 0;
}

void init( void )
{
	if( wiringPiSetup() == -1 )
	{
		exit(1);
	}

	pinMode( SW_R, INPUT );
	pinMode( SW_Y, INPUT );
	pinMode( SW_G, INPUT );
	pinMode( SW_W, INPUT );
	pinMode( LED_R, OUTPUT );
	pinMode( LED_Y, OUTPUT );
	pinMode( LED_G, OUTPUT );

	off();
	blink();

}

void off( void )
{   
	digitalWrite( LED_R, 0 );
	digitalWrite( LED_Y, 0 );
	digitalWrite( LED_G, 0 );
}

void blink( void )
{   
	int i=0;
	    
	while( i < 3 )
	{
		digitalWrite( LED_R, 1 );
		delay(250);
		digitalWrite( LED_R, 0 );

		digitalWrite( LED_Y, 1 );
		delay(250);
		digitalWrite( LED_Y, 0 );

		digitalWrite( LED_G, 1 );
		delay(250);
		digitalWrite( LED_G, 0 );

		i++;
	}   
}

void fail( void )
{
	int i = 0;
	    
	while( i < 3 )
	{
		digitalWrite( LED_R, 1 );
		digitalWrite( LED_Y, 1 );
		digitalWrite( LED_G, 1 );
		delay( 250 );

		digitalWrite( LED_R, 0 );
		digitalWrite( LED_Y, 0 );
		digitalWrite( LED_G, 0 );
		delay( 250 );

		i++;
	}
}

int checkAns( void )
{
	int i = 0;
	while( i < nRound )
	{
		if(nRoundAns[i] != userAns[i])
		{
			fail();
			return 0;
		}
		i++;
	}
	blink();
	return 1;
}

void viewRound( void ){
	int i = 0, roundLight = 0;
	while( i < nRound )
	{
		roundLight = nRoundAns[i];
		if( roundLight == 1 )
		{
			digitalWrite( LED_R, 1 );
		}
		else if( roundLight == 2 )
		{
			digitalWrite( LED_Y, 1 );
		}
		else if( roundLight == 3 )
		{
			digitalWrite( LED_G, 1 );
		}
		delay( 250 );
		off();
		delay( 250 );
		i++;
	}
}
void lights( void ){
	while( nRound < 6 ){
		sem_wait( &semW );
		if( digitalRead( SW_R ) == 0 )
		{   
			sem_post( &semR );
			sem_wait( &semR );
			delay( 250 );
		}
		else if( digitalRead( SW_Y ) == 0 )
		{
			sem_post( &semY );
			sem_wait( &semY );
			delay( 250 );
		}
		else if( digitalRead( SW_G ) == 0 )
		{
			sem_post( &semG );
			sem_wait( &semG );
			delay( 250 );
		}
		sem_post( &semW );
	}
}

void redLight( void )
{
	while( nRound < 6 ){
		sem_wait( &semR );
		digitalWrite(LED_R, 1);
		delay( 250 );
		*( userAns + nLight ) = 1;
		nLight++;
		off();
		sem_post( &semR );
		delay( 250 );
	}
}

void yellowLight( void )
{
	while( nRound < 6 ){
		sem_wait( &semY );
		digitalWrite( LED_Y, 1 );
		delay( 250 );
		*( userAns + nLight ) = 2;
		nLight++;
		off();
		sem_post( &semY );
		delay( 250 );
	}
}

void greenLight( void ){
	while( nRound < 6 ){
		sem_wait( &semG );
		digitalWrite( LED_G, 1 );
		delay( 250 );
		*( userAns + nLight ) = 3;
		nLight++;
		off();
		sem_post( &semG );
		delay( 250 );
	}
}
