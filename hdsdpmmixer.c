/* Very simple Mixer for RME DSP-MADI and maybe other hammerfall dsp 
   (C) 2003 IEM, Winfried Ritsch  (ritsch at iem.at)
   Institute of Electronic Music and Acoustics
   GPL - see Licence.txt

   Use it as library for other programs, or standalone commandline programm

   eg.:compile as program: cc -o hdspmmixer -DMAIN -lm -lc -lasound hdsdpmmixer.c
*/
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>

/* globals */
#define HDSP_VERSION "0.6"
#define HDSP_MAX_CARDS 3
#define HDSP_MAX_NAME_LEN 128

/* Error Codes */
#define HDSP_ERROR_WRONG_IDX    -1
#define HDSP_ERROR_ALSA_OPEN    -2
#define HDSP_ERROR_ALSA_WRITE    -3
#define HDSP_ERROR_ALSA_READ    -4
#define HDSP_ERROR_NO_CARD -5

int get_gain(int idx, int src, int dst);
int set_gain(int idx, int src, int dst, int val);
int find_cards();

static char card_name[HDSP_MAX_CARDS][HDSP_MAX_NAME_LEN];
static int cardid = 0;

int find_cards()
{
  int i;
  char *name;
  int card = HDSP_ERROR_NO_CARD;
  int n = 0;

  printf("Searching mixer for hdspm ...\n");

  while (snd_card_next(&card) >= 0) {
    if (card < 0) {
      break;
    } else {

      snd_card_get_name(card, &name);

      printf("Card %d : %s\n", card, name);
      switch(name) {
        case "Hammerfall DSP":
          snprintf(card_name[n], 6, "hw:%i", n);
          printf("Hammerfall DSP as card %d (%s) found !\n",n,card_name[n]);
          n++;
        case "BCR2000":
          snprintf(card_name[n], 6, "hw:%i", n);
          printf("Hammerfall DSP as card %d (%s) found !\n",n,card_name[n]);
          n++;
      }
    }
  }

  if (!n)
    return HDSP_ERROR_NO_CARD;

  for (i = n; i < HDSP_MAX_CARDS; i++) {
    card_name[i][0] = '\0';
  }

  return n;
}

int set_gain(int idx, int src, int dst, int val)
{
  int err;


  snd_ctl_elem_id_t *id;
  snd_ctl_elem_value_t *ctl;
  snd_ctl_t *handle;

  if(idx >= HDSP_MAX_CARDS || idx < 0)
	return HDSP_ERROR_WRONG_IDX;

  snd_ctl_elem_value_alloca(&ctl);
  snd_ctl_elem_id_alloca(&id);
  snd_ctl_elem_id_set_name(id, "Mixer");
  snd_ctl_elem_id_set_numid(id, 0);
  snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_HWDEP);
  snd_ctl_elem_id_set_device(id, 0);
  snd_ctl_elem_id_set_subdevice(id, 0);
  snd_ctl_elem_id_set_index(id, 0);
  snd_ctl_elem_value_set_id(ctl, id);

  if ((err = snd_ctl_open(&handle, card_name[idx], SND_CTL_NONBLOCK)) < 0) {
    fprintf(stderr, "Alsa error: %s\n", snd_strerror(err));
    return HDSP_ERROR_ALSA_OPEN;
  }

  snd_ctl_elem_value_set_integer(ctl, 0, src);
  snd_ctl_elem_value_set_integer(ctl, 1, dst);
  snd_ctl_elem_value_set_integer(ctl, 2, val);

  if ((err = snd_ctl_elem_write(handle, ctl)) < 0) {
    fprintf(stderr, "Alsa error: %s\n", snd_strerror(err));
    return HDSP_ERROR_ALSA_WRITE;
  }

  val = snd_ctl_elem_value_get_integer(ctl, 2);

  snd_ctl_close(handle);
  return val;
}

int get_gain(int idx, int src, int dst)
{
  int err;
  int val = HDSP_ERROR_NO_CARD;

  snd_ctl_elem_id_t *id;
  snd_ctl_elem_value_t *ctl;
  snd_ctl_t *handle;

  if(idx >= HDSP_MAX_CARDS || idx < 0)
	return HDSP_ERROR_WRONG_IDX;


  snd_ctl_elem_value_alloca(&ctl);
  snd_ctl_elem_id_alloca(&id);
  snd_ctl_elem_id_set_name(id, "Mixer");
  snd_ctl_elem_id_set_numid(id, 0);
  snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_HWDEP);
  snd_ctl_elem_id_set_device(id, 0);
  snd_ctl_elem_id_set_subdevice(id, 0);
  snd_ctl_elem_id_set_index(id, 0);
  snd_ctl_elem_value_set_id(ctl, id);

  if ((err = snd_ctl_open(&handle, card_name[cardid], SND_CTL_NONBLOCK)) < 0) {
    fprintf(stderr, "Alsa error: %s\n", snd_strerror(err));
    return HDSP_ERROR_ALSA_OPEN;
  }

  snd_ctl_elem_value_set_integer(ctl, 0, src);
  snd_ctl_elem_value_set_integer(ctl, 1, dst);

  if ((err = snd_ctl_elem_read(handle, ctl)) < 0) {
    fprintf(stderr, "Alsa error: %s\n", snd_strerror(err));
    return HDSP_ERROR_ALSA_READ;
  }
  val = snd_ctl_elem_value_get_integer(ctl, 2);

  snd_ctl_close(handle);

  return val;
}

#ifdef MAIN

int main(int argc, char *argv[])
{
  int err = 0;

  int src;
  int dst;
  int val;

  /*
  int i;

  for(i=0; i < argc; i++)
    printf(" %s ", argv[i]);
  printf("\n");
  */
  /*
  if((err =find_cards()) < 0)
    return err;
  */

  if(argc < 4){
    printf("\nusage %s <devnr> <src> <dst> [value] \n\n"
	   " devnr ... ALSA-Device eg. for hw:0 is 0 \n"
	   " src   ... inputs 0-63 playback 64-127\n"
	   " dst   ... out channel 0-63\n"
	   "\noptional if wanting to set a value:\n\n"
	   " value ... gain 0=0 (mute), 32768=1 (unitGain), 65535 = max\n\n",
	   argv[0]);

    if(find_cards() < 0)
	    puts("No Hammerfall DSP MADI card found.");
    puts("");
    printf("Version %s, 2003 - IEM, winfried ritsch\n",HDSP_VERSION);
    return -1;
  }

  cardid = atoi(argv[1]);
  snprintf(card_name[cardid], 6, "hw:%i", cardid);

  src = atoi(argv[2]);
  dst = atoi(argv[3]);

  if(argc == 4){

    printf(" Get Mixer from %d to %d : %d \n",src,dst,
	   get_gain(cardid,src,dst));

    return 0;
  }

  /* arg is 5 */
  val =  atoi(argv[4]);

  if((err = set_gain(cardid,src,dst,val)) < 0 )
    printf("Error: Could not set mixer from %d to %d gain %d:%s \n", 
	   src,dst,val,snd_strerror(err));
  else
    printf("Set Mixer from %d to %d : %d \n",src,dst,val);
	   

  return err;
}
#endif
