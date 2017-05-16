// Speak And Spell Simulator for Linux / Raspberry Pi
// (it uses audio samples provided by Maxim Porges)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/stat.h>

//----------------------------------------------------------------------
// read keyboard, returns -1 if no key, otherwise key code.
int getkey() {
    int character;
    struct termios orig_term_attr;
    struct termios new_term_attr;

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    /* read a character from the stdin stream without blocking */
    /*   returns EOF (-1) if no character is available */
    character = fgetc(stdin);

    /* restore the original terminal attributes */
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

    return character;
}

//----------------------------------------------------------------------
main(int argc, char *argv[])
{
  FILE *fp, *fp_stt;
  char words[200][20],typedword[1000];
  int word,wordcount=0,i;
  char sampword[100];
  char sampsay[100];
  int correct,letter,key,done;
  int spellword=0,correctword=0,wrongword=0;
  int nokb=0; // run without keyboard?
  
  if (argc==2 && strncmp(argv[1],"nokb",5)==0) nokb=1;

  fp=fopen("words","r");
  while(!feof(fp)) {
    fscanf(fp,"%s\n",&words[wordcount][0]); wordcount++;
  }
  fclose(fp);
  system("aplay -q samples/tones/melody2.wav");
  
  srand(time(NULL));   // should only be called once
  while(1) {
    word=rand()%wordcount;
    
    //printf("word=%i\n",word);
    sprintf(sampsay,"aplay -q samples/other_words/spell%i.wav",spellword);
    spellword=rand()%5;
    system(sampsay);
    sprintf(sampword,"aplay -q samples/words/%s.wav",words[word]);
    system(sampword);
    // now read characters....
    done=0; letter=0;
  
    while(!done) {  // keep reading keys till ENTER pressed...
      if (nokb) usleep(300000); // to avoid recognizer feedback loops
      if (nokb==0) key=getkey(); else {
        // flush translator output...
        system("rm -f /dev/shm/stt");
        while(!(fp_stt=fopen("/dev/shm/stt","r"))) usleep(10000);
        key=fgetc(fp_stt); fclose(fp_stt);
      }
      if (key>='a' && key<='z') {
        printf("\ngot %c\n",key);
        sprintf(sampsay,"aplay -q samples/alphabet/%c.wav",key);
        system(sampsay);
        typedword[letter]=key; letter++;
      }
      if (key==127 || key=='B') { // backspace pressed
        printf("\nbackspace\n");
        sprintf(sampsay,"aplay -q samples/words/remove.wav",key);
        system(sampsay);
        if (letter>0) letter--;
      }
      if (key==10 || key=='R') done=1; // ENTER key pressed
    }
    if (letter==strnlen(words[word],1000)) {
      correct=1;
      for (i=0; i<letter; i++)
        if (typedword[i]!=words[word][i]) correct=0;
    } else correct=0;
    if (correct) {
      printf("correct!\n");
      correctword=rand()%5;
      sprintf(sampsay,"aplay -q samples/other_words/correct%i.wav",correctword);
      system(sampsay);
    } else {
      printf("wrong! it was %s\n",words[word]);
      wrongword=rand()%2;
      sprintf(sampsay,"aplay -q samples/other_words/incorrect%i.wav",wrongword); system(sampsay);
      sprintf(sampsay,"aplay -q samples/other_words/the_correct.wav");  system(sampsay);
      system(sampword);
      sprintf(sampsay,"aplay -q samples/other_words/is.wav");  system(sampsay);
      for (letter=0; letter<strlen(words[word]); letter++) {
        sprintf(sampsay,"aplay -q samples/alphabet/%c.wav",words[word][letter]);
        system(sampsay);
      }   
    }
    sleep(1); // slight pause before next word.
  }
}
