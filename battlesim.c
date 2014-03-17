#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

typedef enum { FALSE = 0, TRUE = 1 } Bool;
typedef Bool* BoolPtr;

//used to help matrix maps for the game board
#define ARRAYSIZE(x)  (sizeof(x)/sizeof(*(x)))

//Control of how many simulations are done, and how many times its done
#define NUM_OF_SIMS 1000
#define NUM_OF_RUNS 1

pthread_mutex_t mutex1;
pthread_cond_t mutex1c;

//mutex1c sends a flag to each simulation thread to determine if it should continue or terminate
Bool cont = FALSE;
unsigned int score1 = 0;
unsigned int score2 = 0;
void battleship(void *battleshiparg);

pthread_mutex_t mutex2;
pthread_cond_t mutex2c;

Bool MainThreadWaiting = FALSE;

void *simulation(void *simulationx){
  unsigned int X;
  long unsigned int sim_num = (long unsigned int)simulationx;
  
  // Enter a waiting state
  pthread_mutex_lock(&mutex1);
  pthread_cond_wait(&mutex1c, &mutex1);
  while ( TRUE ) {
    if ( cont ) {
      pthread_mutex_unlock(&mutex1);
    }
    else {
      pthread_mutex_unlock(&mutex1);
      pthread_exit(NULL);
    }


	battleship(NULL);


    while ( TRUE ) {
      pthread_mutex_lock(&mutex2);
      if ( MainThreadWaiting == TRUE ) {
        MainThreadWaiting = FALSE;
        break;
      }
      pthread_mutex_unlock(&mutex2);
    }
    pthread_mutex_lock(&mutex1);
    pthread_cond_signal(&mutex2c);
    pthread_mutex_unlock(&mutex2);
    pthread_cond_wait(&mutex1c, &mutex1);
  }
}


void battleship(void *battleshiparg){

const char loadmap1[] = "map1.csv";

const char loadmap2[] = "map2.csv";

const char filepath[] = "record.txt";
 
 int map1[10][10];
 int map2[10][10];
 int player1 = 17;
 int player2 = 17;
 int temp;
 int b, c, m, n;
 unsigned int flag = 0;


//Open map file 1
   FILE *map1open = fopen(loadmap1, "r");
   if ( map1open )
   {  
      size_t i, j, k;
      char buffer[BUFSIZ], *point;
      
    //Read each line of the file
      for ( i = 0; fgets(buffer, sizeof buffer, map1open); ++i )
      {
        
	//place the comma seperated values into the map1 array  
         for ( j = 0, point = buffer; j < ARRAYSIZE(*map1); ++j, ++point )
         {
            map1[i][j] = (int)strtol(point, &point, 10);
         }
      }
      fclose(map1open);
}


//Open map file 2
   FILE *map2open = fopen(loadmap2, "r");
   if ( map2open )
   {  
      size_t i, j, k;
      char buffer[BUFSIZ], *point;
      
    //Read each line of the file
      for ( i = 0; fgets(buffer, sizeof buffer, map2open); ++i )
      {
        
	//place the comma seperated values into the map1 array  
         for ( j = 0, point = buffer; j < ARRAYSIZE(*map2); ++j, ++point )
         {
            map2[i][j] = (int)strtol(point, &point, 10);
         }
      }
      fclose(map2open);
}

//Continue the game until one person wins
	while((player1 != 0) && (player2 != 0)){
//player 1 turn aka uniform linear search turn
	if(flag == 0){
	temp = map1[b][c];	
		if(temp == 1){
		player1--;
		map1[b][c] = 0;
		}
		else{
		c++;
			if(c == 10){
			c = 0;
			b++;
			}
		flag = 1;
		}
	}
//player 2 turn aka non uniform search turn
	else{
	 m = rand() % 10;
 	 n = rand() % 10;
	temp = map2[m][n];
		if(temp == 1){
		player2--;
		map2[m][n] = 2;
		}
		else if(temp == 2){
		}
		else{
		map2[m][n] = 2;
		flag = 0;
		}
	
	}


}
//end of while loop



//print out to record of wins list
if(player1 == 0){
score1++;
 FILE *file = fopen(filepath, "ab");
    if (file != NULL)
    {
        fputs("Player 1 Wins\n", file);
        fclose(file);
    }
}

else if(player2 == 0){
score2++;
 FILE *file = fopen(filepath, "ab");
    if (file != NULL)
    {
        fputs("Player 2 Wins\n", file);
        fclose(file);
    }
}


}


int main(){
  long unsigned int sim_number;
  unsigned int X;
  unsigned int Y;
  
  // Initialize all of the thread related objects.
  pthread_t Threads[NUM_OF_SIMS];
  pthread_attr_t attr;
  
  pthread_mutex_init(&mutex2, NULL);
  pthread_cond_init (&mutex2c, NULL);
  
  pthread_mutex_init(&mutex1, NULL);
  pthread_cond_init (&mutex1c, NULL);
  
  //Make all threads joinable	
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  
  // Create all of the Simulations here.
  for ( sim_number = 0; sim_number < NUM_OF_SIMS; sim_number++ ) pthread_create(&Threads[sim_number], &attr, simulation, (void *)sim_number);
  
  // Allow time for simulation threads to start up
  sleep(1);
  
  // Send out the signal to begin working
  for ( X = 0; X < NUM_OF_RUNS; X++ ) {
    pthread_mutex_lock(&mutex1);
    cont = TRUE;
    pthread_mutex_lock(&mutex2);
    pthread_cond_broadcast(&mutex1c);
    pthread_mutex_unlock(&mutex1);
    
    for ( Y = 0; Y < NUM_OF_SIMS; Y++ ) {
      MainThreadWaiting = TRUE;
      pthread_cond_wait(&mutex2c, &mutex2);
    }
    pthread_mutex_unlock(&mutex2);
  }
  
  
  pthread_mutex_lock(&mutex1);
  // setting the end condition
  cont = FALSE;
  pthread_cond_broadcast(&mutex1c);
  pthread_mutex_unlock(&mutex1);

  // Wait for all threads to complete, and then join with them.
  for ( X = 0; X < NUM_OF_SIMS; X++ ) {
    pthread_join(Threads[X], NULL);
  }

	if(score1 > score2){
	printf("%u : %u in Player 1's favor", score1, score2);
	}
	else if(score1 < score2){
	printf("%u : %u in Player 2's favor", score2, score1);
	}
	else{
	printf("Tie Game, Players are equal 500/500");
	}
  
  // Clean up
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&mutex2);
  pthread_cond_destroy(&mutex2c);
  pthread_mutex_destroy(&mutex1);
  pthread_cond_destroy(&mutex1c);
  pthread_exit (NULL);
}
