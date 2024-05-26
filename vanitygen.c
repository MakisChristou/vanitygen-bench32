#include <stdio.h>

// Basic secp256k1 config
#define USE_BASIC_CONFIG
#define ECMULT_GEN_PREC_BITS 2

#include "src/basic-config.h"
#include "src/secp256k1.c"


#include "externs.h"
#include "segwit_addr.h"

#include <pthread.h>
#include <time.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>


// Global variables
int debug = 0;
int threads = -1;
int output = 0;
char *pattern = "bcq1test"; // Pattern to match 
char *hrp = "bc"; // Different for each coin
char *output_file;
int update_time = 5; // Update screen every x seconds
int hex_priv = 0; // 1 for hex private keys, else WIF

// Stolen from supervanitygen
void announce_result(int found, const u8 result[52], int hex_priv)
{
  align8 u8 priv_block[64], pub_block[64], cksum_block[64];
  align8 u8 wif[64], checksum[32];
  int j;

  /* Convert Private Key to WIF */

  /* Set up sha256 block for hashing the private key; length of 34 bytes */
  sha256_prepare(priv_block, 34);
  priv_block[0]=0x80;
  memcpy(priv_block+1, result, 32);
  priv_block[33]=0x01;  /* 1=Compressed Public Key */

  /* Set up checksum block; length of 32 bytes */
  sha256_prepare(cksum_block, 32);

  /* Compute checksum and copy first 4-bytes to end of private key */
  sha256_hash(cksum_block, priv_block);
  sha256_hash(checksum, cksum_block);
  memcpy(priv_block+34, checksum, 4);

  if(hex_priv == 1)
  {	
	  printf("Private Key:   ");
	  for(int i = 0; i < 32; i++)
	  	printf("%02x", result[i]);
	  printf("\n");
  }
  else
  {
	  b58enc(wif, priv_block, 38);
	  printf("Private Key:   %s\n", wif);
  }
  
  

//   /* Convert Public Key to Compressed WIF */

//   /* Set up sha256 block for hashing the public key; length of 21 bytes */
//   sha256_prepare(pub_block, 21);
//   memcpy(pub_block+1, result+32, 20);

//   /* Compute checksum and copy first 4-bytes to end of public key */
//   sha256_hash(cksum_block, pub_block);
//   sha256_hash(checksum, cksum_block);
//   memcpy(pub_block+21, checksum, 4);

//   b58enc(wif, pub_block, 25);
 
//   printf("Address:       %s\n", wif);
}


// Run this on invalid user input
void print_usage()
{
	printf("vanitygen -p <PATTERN> -t <THREADS> -o <FILE>\n");
}

// Difficulty = 1/{valid pattern space}
double get_difficulty(char* pattern, char* hrp)
{
	int start = strlen(hrp)+2;
	int length = strlen(pattern);

	//33^(length)
	double pattern_space = pow(33,(length-start));

	printf("Pattern Space = %lf\n",pattern_space);

}

extern void check_pattern(char* pattern);

// Use getopt to parse cli arguments
void parse_arguments(int argc, char** argv)
{

	if(argc < 2)
	{
		print_usage();
		exit(1);
	}

	int opt;
	while((opt = getopt(argc,argv, "hp:t:d")) != -1)
	{
		//Print Help Message
		if(opt == 'h')
		{
			print_usage();
			exit(1);
		}
		// Output results in a file
		else if(opt == 'o')
		{
			output_file = optarg;
		}
		// Choose number of threads
		else if(opt == 't')
		{
			threads = atoi(optarg);
		}
		// Choose pattern
		else if(opt == 'p')
		{
			check_pattern(optarg);
			pattern = optarg;
		}
		else if(opt == 'd')
		{
			hex_priv = 1;
		}
		else
		{
			exit(1); // exit on wrong argument
		}
		
	}
}


// Make sure user provided pattern is correct
void check_pattern(char* pattern)
{
	if(pattern[0] != 'b' || pattern[1] != 'c' ||pattern[2] != '1' ||pattern[3] != 'q')
	{
		printf("Bitcoin address starts with bc1q\n");
		exit(1);
	}
	

	// Check if pattern is valid
	for(int i = strlen(hrp)+2; i < strlen(pattern); i++)
	{
		// if(pattern[i] == '1' || pattern[i] == 'b' || pattern[i] == 'i' || pattern[i] == 'o')
		// {
		// 	print_patterns();
		// 	exit(1);
		// }
		if(pattern[i] != 'a' && pattern[i] != 'c' && pattern[i] != 'd'&& pattern[i] != 'e'&& pattern[i] != 'f'&& pattern[i] != 'g'&& pattern[i] != 'h' && pattern[i] != 'j'&& pattern[i] != 'k'&& pattern[i] != 'l'&& pattern[i] != 'm'&& pattern[i] != 'n' && pattern[i] != 'p'&& pattern[i] != 'q'&& pattern[i] != 'r'&& pattern[i] != 's'&& pattern[i] != 't'&& pattern[i] != 'u'&& pattern[i] != 'v'&& pattern[i] != 'w'&& pattern[i] != 'x'&& pattern[i] != 'y'&& pattern[i] != 'z'&& pattern[i] != '2'&& pattern[i] != '3'&& pattern[i] != '4'&& pattern[i] != '5'&& pattern[i] != '6'&& pattern[i] != '7'&& pattern[i] != '8'&& pattern[i] != '9'&& pattern[i] != '0')
		{
			printf("Invalid Pattern!\n");
			printf("Valid characters are: acdefghjklmnpqrstuvwxyz023456789\n");
			exit(1);
		}
	}
}


// Address generation code
void* vanity_engine(void *vargp)
{
	int threadid = *(int *)vargp;

	// Declare Secp256k1 Stuff
	secp256k1_context *sec_ctx;
	unsigned char sha_block[64], rmd_block[64], ScriptPubKey[20];
	u64 privkey[4]; // private key binary
	int i, k, fd, len; //for udev random
	secp256k1_pubkey public_key; // public key object
	unsigned char compressed_pubkey[33]; // compressed public key binary
	char output[93]; //bech32 encoding output
	const uint8_t *witprog;
	witprog = ScriptPubKey;
	size_t witprog_len = 20;
	clock_t start, end;
	clock_t start_elapsed, end_elapsed;
    double iteration_time;
	double total_time;
	unsigned long long int iteration = 0;
	double iterations_per_second = 0;
	int flag = 1;

	char actual_pattern[93];

	/* Initialize the secp256k1 context */
	sec_ctx=secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);

	// Generate a random private key. Specifically, any 256-bit number from 0x1
	// to 0xFFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFE BAAE DCE6 AF48 A03B BFD2 5E8C
	// D036 4140 is a valid private key.

	for(int i = 4; i < strlen(pattern); i++)
	{
		actual_pattern[i-4] = pattern[i];
	}

	//printf("%s\n",actual_pattern);

	// To calculate total time
	start_elapsed = clock();

	again:

    start = clock();
	
	// Generate private key
	if((fd=open("/dev/urandom", O_RDONLY|O_NOCTTY)) == -1) {
		perror("/dev/urandom");
		return NULL;
	}

	// Stolen from supervanitygen
	/* Use 32 bytes from /dev/urandom as starting private key */
	do {
	if((len=read(fd, privkey, 32)) != 32) {
		if(len != -1)
		errno=EAGAIN;
		perror("/dev/urandom");
		return NULL;
	}
	} while(privkey[0]+1 < 2);  /* Ensure only valid private keys */

	close(fd);

	// Generate Public Key from Private Key
	secp256k1_ec_pubkey_create(sec_ctx,&public_key,&privkey);

	// Generate Compressed Public Key
	size_t  var = 33;   		/* actual variable declaration */
	size_t  *outputlen;        /* pointer variable declaration */
	outputlen = &var;  		/* store address of var in pointer variable*/

	int result = secp256k1_ec_pubkey_serialize(sec_ctx, compressed_pubkey, outputlen, &public_key, SECP256K1_EC_COMPRESSED);
	
	// Double Hash Compressed public key
	SHA256(compressed_pubkey, 33, rmd_block);
	RIPEMD160(rmd_block, 32, ScriptPubKey);


	int convert_bech32 = segwit_addr_encode(output,hrp,0,witprog, witprog_len);

	// If convertion fails exit gracefully
	if(convert_bech32 == 0)
	{
		printf("Bech32 convertion failed\n");
		exit(1);
	}


	// Thread 0 updates the screen
	if(threadid == 0)
	{
		end = clock();
		end_elapsed = end;
		total_time = ((double) (end_elapsed - start_elapsed)) / CLOCKS_PER_SEC; total_time = total_time / threads;
		iteration_time = ((double) (end - start)) / CLOCKS_PER_SEC; iteration_time = iteration_time / threads;
		iteration = iteration + threads;
		//iterations_per_second = 1.0*iteration/total_time;
		iterations_per_second = threads*1.0/iteration_time;

		int pattern_length = strlen(pattern)-4;
		double num_of_patterns = pow(33,pattern_length);
		double eta = iteration_time*num_of_patterns/threads;
		
		int total_time_rounded = (int)total_time;
		int days = (int)total_time_rounded/60/60/24;
		int hours =  ((total_time_rounded)/60/60) % 24;
		int minutes = ((total_time_rounded)/60) % (60);
		int seconds = (total_time_rounded) % 60;

		if((flag == 1) && ((total_time_rounded % update_time) == 0))
		{
			flag = 0;
			// Seconds
			if(eta < 2*60)
			{
				printf("[%02d:%02d:%02d:%02d][%d Kkey/s][Total %d][Eta %0.0lf sec]\n",days,hours,minutes,seconds,(int)iterations_per_second/1000,iteration,eta);
			}
			// Minutes
			else if(eta < 2*60*60)
			{
				printf("[%02d:%02d:%02d:%02d][%d Kkey/s][Total %d][Eta %0.0lf min]\n",days,hours,minutes,seconds,(int)iterations_per_second/1000,iteration,eta/60);
			}
			// Hours
			else if(eta < 2*60*60*24)
			{
				printf("[%02d:%02d:%02d:%02d][%d Kkey/s][Total %d][Eta %0.0lf hours]\n",days,hours,minutes,seconds,(int)iterations_per_second/1000,iteration,eta/60/60);
			}
			// Days
			else if(eta < 2*60*60*24*365*2)
			{
				printf("[%02d:%02d:%02d:%02d][%d Kkey/s][Total %d][Eta %0.0lf days]\n",days,hours,minutes,seconds,(int)iterations_per_second/1000,iteration,eta/60/60/24);
			}
			else
			{
				printf("[%02d:%02d:%02d:%02d][%d Kkey/s][Total %d][Eta %0.0lf years]\n",days,hours,minutes,seconds,(int)iterations_per_second/1000,iteration,eta/60/60/24/365);
			}
		}
		else if(((total_time_rounded % update_time) != 0) && (flag == 0))
		{
			flag = 1;
		}
	}	
		
	// Check if pattern matches
	for(int i = 0; i < strlen(actual_pattern); i++)
	{
		if(!(actual_pattern[i] == output[i+4]))
		{
			goto again;
		}
	}

	printf("\n");

	// Check if valid private key
	int valid_private_key = secp256k1_ec_seckey_verify(sec_ctx,&privkey);

	if(valid_private_key)
	{
		// Print WIF or HEX private key
		announce_result(1, privkey, hex_priv);

		// Print Segwit Address
		printf("Address:       %s\n",output);
		exit(0);
	}
	else
	{
		printf("Invalid private key!\n");
		exit(1);
	}
}

int main(int argc, char** argv)
{
	parse_arguments(argc,argv);

	// By default use all available threads
	if(threads == -1)
		threads = get_num_cpus();


	printf("Starting %d threads\n",threads);
	printf("Pattern: %s\n",pattern);

 	int noOfThread = threads;
    pthread_t thread_id[noOfThread];
    int i;
    int status;

    for(i=0;i<noOfThread;i++)
    	pthread_create (&thread_id[i], NULL , &vanity_engine, (void*)&i);
    
    for(i=0;i<noOfThread;i++)
        pthread_join(thread_id[i],NULL);  

	return 0;
}

