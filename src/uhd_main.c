#include "unhexdump.h"

struct uhd uhd={0};

/* The main event, all in memory.
 */
 
static int uhd_unhexdump() {
  int srcp=0,lineno=1,comment=0,hi=-1;
  while (srcp<uhd.srcc) {
    
    if (uhd.src[srcp]==0x0a) {
      if (hi>=0) {
        fprintf(stderr,"%s:%d: Uneven count of hex digits.\n",uhd.srcpath,lineno);
        return -2;
      }
      lineno++;
      srcp++;
      comment=0;
      continue;
    }
    
    if (comment) {
      srcp++;
      continue;
    }
    if (uhd.src[srcp]=='#') {
      comment=1;
      srcp++;
      continue;
    }
    
    if (uhd.src[srcp]<=0x20) {
      srcp++;
      continue;
    }
    
    int digit=-1;
    uint8_t ch=uhd.src[srcp++];
         if ((ch>='0')&&(ch<='9')) digit=ch-'0';
    else if ((ch>='a')&&(ch<='f')) digit=ch-'a'+10;
    else if ((ch>='A')&&(ch<='F')) digit=ch-'A'+10;
    else {
      fprintf(stderr,"%s:%d: Unexpected character '%c' in hex dump.\n",uhd.srcpath,lineno,ch);
      return -2;
    }
    
    if (hi<0) {
      hi=digit;
    } else {
      if (uhd.dstc>=uhd.dsta) {
        int na=uhd.dsta+4096;
        void *nv=realloc(uhd.dst,na);
        if (!nv) return -1;
        uhd.dst=nv;
        uhd.dsta=na;
      }
      uhd.dst[uhd.dstc++]=(hi<<4)|digit;
      hi=-1;
    }
  }
  return 0;
}

/* Acquire input.
 * Do not assume it's a regular file.
 */
 
static int uhd_read_src() {
  int fd;
  if (uhd.srcpath) {
    if ((fd=open(uhd.srcpath,O_RDONLY))<0) {
      fprintf(stderr,"%s: Failed to open file: %m\n",uhd.srcpath);
      return -2;
    }
  } else {
    fd=STDIN_FILENO;
    uhd.srcpath="<stdin>";
  }
  int srca=32768;
  if (!(uhd.src=malloc(srca))) return -1;
  for (;;) {
    if (uhd.srcc>=srca) {
      srca<<=1;
      if (srca>UHD_INPUT_SANITY_LIMIT) {
        fprintf(stderr,"%s: Exceeded input sanity limit %d b.\n",uhd.srcpath,UHD_INPUT_SANITY_LIMIT);
        if (fd!=STDIN_FILENO) close(fd);
        return -2;
      }
      void *nv=realloc(uhd.src,srca);
      if (!nv) {
        if (fd!=STDIN_FILENO) close(fd);
        return -1;
      }
      uhd.src=nv;
    }
    int err=read(fd,uhd.src+uhd.srcc,srca-uhd.srcc);
    if (err<0) {
      fprintf(stderr,"%s: Read failed: %m\n",uhd.srcpath);
      if (fd!=STDIN_FILENO) close(fd);
      return -2;
    }
    if (!err) break;
    uhd.srcc+=err;
  }
  if (fd!=STDIN_FILENO) close(fd);
  return 0;
}

/* Write output.
 */
 
static int uhd_write_dst() {
  int fd;
  if (uhd.dstpath) {
    if ((fd=open(uhd.dstpath,O_WRONLY|O_CREAT|O_TRUNC,0666))<0) {
      fprintf(stderr,"%s: Failed to create file: %m\n",uhd.dstpath);
      return -2;
    }
  } else {
    fd=STDOUT_FILENO;
    uhd.dstpath="<stdout>";
  }
  int dstp=0;
  while (dstp<uhd.dstc) {
    int err=write(fd,uhd.dst+dstp,uhd.dstc-dstp);
    if (err<=0) {
      fprintf(stderr,"%s: Write failed: %m\n",uhd.dstpath);
      if (fd!=STDOUT_FILENO) {
        close(fd);
        unlink(uhd.dstpath);
      }
      return -2;
    }
    dstp+=err;
  }
  if (fd!=STDOUT_FILENO) close(fd);
  return 0;
}

/* --help
 */
 
static void uhd_print_help() {
  fprintf(stderr,
    "\n"
    "Usage: %s [-oOUTPUT] [INPUT]\n"
    "\n"
    "Omit either file to use stdin or stdout.\n"
    "stdout must not be a TTY.\n"
    "\n"
  ,uhd.exename);
}

/* Main.
 */
 
int main(int argc,char **argv) {

  if ((argc>=1)&&argv&&argv[0]) uhd.exename=argv[0];
  else uhd.exename="unhexdump";
  
  int argi=1; while (argi<argc) {
    const char *arg=argv[argi++];
    if (!arg||!arg[0]) continue;
    if (!strcmp(arg,"--help")) {
      uhd_print_help();
      return 0;
    }
    if ((arg[0]=='-')&&(arg[1]=='o')) {
      if (uhd.dstpath) {
        fprintf(stderr,"%s: Multiple output paths.\n",uhd.exename);
        return 1;
      }
      if (arg[2]) uhd.dstpath+=2;
      else if ((argi<argc)&&argv[argi]&&argv[argi][0]&&(argv[argi][0]!='-')) uhd.dstpath=argv[argi++];
      else {
        uhd_print_help();
        return 1;
      }
    } else if (arg[0]=='-') {
      fprintf(stderr,"%s: Unexpected argument '%s'\n",uhd.exename,arg);
      return 1;
    } else {
      if (uhd.srcpath) {
        fprintf(stderr,"%s: Multiple input paths.\n",uhd.exename);
        return 1;
      }
      uhd.srcpath=arg;
    }
  }
  if (!uhd.dstpath&&isatty(STDOUT_FILENO)) {
    fprintf(stderr,"%s: Refusing to output binary data to a TTY. Please redirect output or specify file as '-oPATH'.\n",uhd.exename);
    return 1;
  }
  
  int err=uhd_read_src();
  if (err<0) {
    if (err!=-2) fprintf(stderr,"%s: Unspecified error reading input.\n",uhd.exename);
    return 1;
  }
  
  if ((err=uhd_unhexdump())<0) {
    if (err!=-2) fprintf(stderr,"%s: Unspecified error parsing hex dump.\n",uhd.srcpath);
    return 1;
  }
  
  if ((err=uhd_write_dst())<0) {
    if (err!=-2) fprintf(stderr,"%s: Unspecified error writing %d-byte output.\n",uhd.dstpath?uhd.dstpath:"<stdout>",uhd.dstc);
    return 1;
  }
  
  return 0;
}
