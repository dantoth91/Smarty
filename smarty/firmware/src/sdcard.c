/*
    Smarty - Copyright (C) 2014
    GAMF MegaLux Team              
*/

#include "sdcard.h"

#include "ff.h"

/*
 * SDIO configuration.
 */
static const SDCConfig sdccfg = {
  0
};

#define SDC_BURST_SIZE      8 /* how many sectors reads at once */
static uint8_t outbuf[MMCSD_BLOCK_SIZE * SDC_BURST_SIZE + 1];
static uint8_t  inbuf[MMCSD_BLOCK_SIZE * SDC_BURST_SIZE + 1];

/**
 * @brief FS object.
 */
static FATFS sdcard_filesystem;

/* FS mounted and ready.*/
static bool_t sdcard_fs_ready;

/* Generic large buffer.*/
uint8_t sdcard_fbuff[1024];

/*===========================================================================*/
/* Card insertion monitor.                                                   */
/*===========================================================================*/

/**
 * @brief   Insertion monitor function.
 * @note    Not supported.
 * 
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
bool_t sdc_lld_is_card_inserted(SDCDriver *sdcp) {

  (void)sdcp;
  return TRUE;
}

/**
 * @brief   Protection detection.
 * @note    Not supported.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
bool_t sdc_lld_is_write_protected(SDCDriver *sdcp) {

  (void)sdcp;
  return FALSE;
}

/*===========================================================================*/
/* SDC related.                                                            */
/*===========================================================================*/


void fillbuffer(uint8_t pattern, uint8_t *b){
  uint32_t i = 0;
  for (i=0; i < MMCSD_BLOCK_SIZE * SDC_BURST_SIZE; i++)
    b[i] = pattern;
}

void fillbuffers(uint8_t pattern){
  fillbuffer(pattern, inbuf);
  fillbuffer(pattern, outbuf);
}

static FRESULT scan_files(BaseSequentialStream *chp, char *path) {
  FRESULT res;
  FILINFO fno;
  DIR dir;
  int i;
  char *fn;
#if _USE_LFN
  static char lfn[_MAX_LFN + 1];
  fno.lfname = lfn;
  fno.lfsize = sizeof lfn;
#endif

  res = f_opendir(&dir, path);
  if (res == FR_OK) {
    i = strlen(path);
    for (;;) {
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0)
        break;
      if (fno.fname[0] == '.')
        continue;
#if _USE_LFN
      fn = *fno.lfname ? fno.lfname : fno.fname;
#else
      fn = fno.fname;
#endif
      if (fno.fattrib & AM_DIR) {
        path[i++] = '/';
        strcpy(&path[i], fn);
        res = scan_files(chp, path);
        if (res != FR_OK)
          break;
        path[i] = 0;
      }
      else {
        chprintf(chp, "%s/%s\r\n", path, fn);
      }
    }
  }
  return res;
}

void sdcardInit(void){
  /*
   * Activates the SDC driver using the driver default configuration.
   */
  sdcStart(&SDCD1, &sdccfg);
  
  sdcard_fs_ready = FALSE;
}

sdcarderror_t sdcardMount(void){
  FRESULT err;
  uint32_t clusters;
  FATFS *fsp;

  /*
   * SDC initialization and FS mount.
   */
  if (sdcConnect(&SDCD1)){
    return SDCARD_ERROR_CONNECT;
  }
  err = f_mount(0, &sdcard_filesystem);
  if (err != FR_OK) {
    sdcDisconnect(&SDCD1);
    return SDCARD_ERROR_FSMOUNT;
  }
  err = f_getfree("/", &clusters, &fsp);
  if (err != FR_OK) {
    f_mount(0, NULL);
    sdcDisconnect(&SDCD1);
    return SDCARD_ERROR_READ;
  }
  sdcard_fs_ready = TRUE;
  return SDCARD_OK;
}

sdcarderror_t sdcardUMount(void){
  sdcard_fs_ready = FALSE;
  f_mount(0, NULL);
  sdcDisconnect(&SDCD1);
  return SDCARD_OK;
}

bool_t sdcardIsMounted(void){
  return sdcard_fs_ready;
}

void cmd_tree(BaseSequentialStream *chp, int argc, char *argv[]) {
  FRESULT err;
  uint32_t clusters;
  FATFS *fsp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: tree\r\n");
    return;
  }
  if (!sdcard_fs_ready) {
    chprintf(chp, "File System not mounted\r\n");
    return;
  }
  err = f_getfree("/", &clusters, &fsp);
  if (err != FR_OK) {
    chprintf(chp, "FS: f_getfree() failed\r\n");
    return;
  }
  chprintf(chp,
           "FS: %lu free clusters, %lu sectors per cluster, %lu bytes free\r\n",
           clusters, (uint32_t)sdcard_filesystem.csize,
           clusters * (uint32_t)sdcard_filesystem.csize * (uint32_t)MMCSD_BLOCK_SIZE);
  sdcard_fbuff[0] = 0;
  scan_files(chp, (char *)sdcard_fbuff);
}

void cmd_mountsd(BaseSequentialStream *chp, int argc, char *argv[]) {
  sdcarderror_t ret;
  
  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: mountsd\r\n");
    return;
  }
  
  ret = sdcardMount();
  switch(ret){
    case SDCARD_ERROR_CONNECT:
      chprintf(chp, "SDC: sdcConnect() failed\r\n");
      break;
    case SDCARD_ERROR_FSMOUNT:
      chprintf(chp, "FS: f_mount() failed\r\n");
      break;
    case SDCARD_ERROR_READ:
      chprintf(chp, "FS: f_getfree() failed\r\n");
      break;
    case SDCARD_OK:
      chprintf(chp, "SD card mounted\r\n");
      break;
    default:
      chprintf(chp, "Unknown Error\r\n");
      break;
  }
}

void cmd_umountsd(BaseSequentialStream *chp, int argc, char *argv[]) {
  sdcarderror_t ret;
  
  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: umountsd\r\n");
    return;
  }
  
  ret = sdcardUMount();
  if(ret == SDCARD_OK){
    chprintf(chp, "SD card unmounted\r\n");
  }
  else{
    chprintf(chp, "SD card unmount failed\r\n");
  }
}

void cmd_cat(BaseSequentialStream *chp, int argc, char *argv[]) {
  FRESULT err;
  FIL FileObject;
  TCHAR buf[80];
  TCHAR* res;
 
  if (argc != 1) {
    chprintf(chp, "Usage: cat FILE\r\n");
    return;
  }
  if (!sdcard_fs_ready) {
    chprintf(chp, "File System not mounted\r\n");
    return;
  }
  err = f_open(&FileObject, *argv, FA_READ | FA_OPEN_EXISTING);
  if (err != FR_OK) {
    chprintf(chp, "FS: f_open() failed\r\n");
    return;
  }
  res = f_gets(buf, 80, &FileObject);
  while (res != 0) {
    chprintf(chp, "%s", buf);
    res = f_gets(buf, 80, &FileObject);
  }
  err = f_close(&FileObject);
  if (err != FR_OK) {
    chprintf(chp, "FS: f_close() failed\r\n");
    return;
  }
}

void cmd_sdiotest(BaseSequentialStream *chp, int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  uint32_t i = 0;

  chprintf(chp, "Trying to connect SDIO... ");
  chThdSleepMilliseconds(100);

  if (!sdcConnect(&SDCD1)) {

    chprintf(chp, "OK\r\n");
    chprintf(chp, "*** Card CSD content is: ");
    chprintf(chp, "%X %X %X %X \r\n", (&SDCD1)->csd[3], (&SDCD1)->csd[2],
                                      (&SDCD1)->csd[1], (&SDCD1)->csd[0]);

    chprintf(chp, "Single aligned read...");
    chThdSleepMilliseconds(100);
    if (sdcRead(&SDCD1, 0, inbuf, 1))
      chSysHalt();
    chprintf(chp, " OK\r\n");
    chThdSleepMilliseconds(100);


    chprintf(chp, "Single unaligned read...");
    chThdSleepMilliseconds(100);
    if (sdcRead(&SDCD1, 0, inbuf + 1, 1))
      chSysHalt();
    if (sdcRead(&SDCD1, 0, inbuf + 2, 1))
      chSysHalt();
    if (sdcRead(&SDCD1, 0, inbuf + 3, 1))
      chSysHalt();
    chprintf(chp, " OK\r\n");
    chThdSleepMilliseconds(100);


    chprintf(chp, "Multiple aligned reads...");
    chThdSleepMilliseconds(100);
    fillbuffers(0x55);
    /* fill reference buffer from SD card */
    if (sdcRead(&SDCD1, 0, inbuf, SDC_BURST_SIZE))
      chSysHalt();
    for (i=0; i<1000; i++){
      if (sdcRead(&SDCD1, 0, outbuf, SDC_BURST_SIZE))
        chSysHalt();
      if (memcmp(inbuf, outbuf, SDC_BURST_SIZE * MMCSD_BLOCK_SIZE) != 0)
        chSysHalt();
    }
    chprintf(chp, " OK\r\n");
    chThdSleepMilliseconds(100);


    chprintf(chp, "Multiple unaligned reads...");
    chThdSleepMilliseconds(100);
    fillbuffers(0x55);
    /* fill reference buffer from SD card */
    if (sdcRead(&SDCD1, 0, inbuf + 1, SDC_BURST_SIZE))
      chSysHalt();
    for (i=0; i<1000; i++){
      if (sdcRead(&SDCD1, 0, outbuf + 1, SDC_BURST_SIZE))
        chSysHalt();
      if (memcmp(inbuf, outbuf, SDC_BURST_SIZE * MMCSD_BLOCK_SIZE) != 0)
        chSysHalt();
    }
    chprintf(chp, " OK\r\n");
    chThdSleepMilliseconds(100);

    /**
     * Now perform someFS tests.
     */

    FRESULT err;
    uint32_t clusters;
    FATFS *fsp;
    FIL FileObject;
    uint32_t bytes_written;
    uint32_t bytes_read;
    FILINFO filinfo;
    uint8_t teststring[] = {"This is test file\r\n"};

    chprintf(chp, "Register working area for filesystem... ");
    chThdSleepMilliseconds(100);
    err = f_mount(0, &sdcard_filesystem);
    if (err != FR_OK){
      chSysHalt();
    }
    else{
      sdcard_fs_ready = TRUE;
      chprintf(chp, "OK\r\n");
    }

    chprintf(chp, "Mount filesystem... ");
    chThdSleepMilliseconds(100);
    err = f_getfree("/", &clusters, &fsp);
    if (err != FR_OK) {
      chSysHalt();
    }
    chprintf(chp, "OK\r\n");
    chprintf(chp,
             "FS: %lu free clusters, %lu sectors per cluster, %lu bytes free\r\n",
             clusters, (uint32_t)sdcard_filesystem.csize,
             clusters * (uint32_t)sdcard_filesystem.csize * (uint32_t)MMCSD_BLOCK_SIZE);


    chprintf(chp, "Create file \"chtest.txt\"... ");
    chThdSleepMilliseconds(100);
    err = f_open(&FileObject, "0:chtest.txt", FA_WRITE | FA_OPEN_ALWAYS);
    if (err != FR_OK) {
      chSysHalt();
    }
    chprintf(chp, "OK\r\n");
    chprintf(chp, "Write some data in it... ");
    chThdSleepMilliseconds(100);
    err = f_write(&FileObject, teststring, sizeof(teststring), (void *)&bytes_written);
    if (err != FR_OK) {
      chSysHalt();
    }
    else
      chprintf(chp, "OK\r\n");

    chprintf(chp, "Close file \"chtest.txt\"... ");
    err = f_close(&FileObject);
    if (err != FR_OK) {
      chSysHalt();
    }
    else
      chprintf(chp, "OK\r\n");

    chprintf(chp, "Check file size \"chtest.txt\"... ");
    err = f_stat("0:chtest.txt", &filinfo);
    chThdSleepMilliseconds(100);
    if (err != FR_OK) {
      chSysHalt();
    }
    else{
      if (filinfo.fsize == sizeof(teststring))
        chprintf(chp, "OK\r\n");
      else
        chSysHalt();
    }

    chprintf(chp, "Check file content \"chtest.txt\"... ");
    err = f_open(&FileObject, "0:chtest.txt", FA_READ | FA_OPEN_EXISTING);
    chThdSleepMilliseconds(100);
    if (err != FR_OK) {
      chSysHalt();
    }
    uint8_t buf[sizeof(teststring)];
    err = f_read(&FileObject, buf, sizeof(teststring), (void *)&bytes_read);
    if (err != FR_OK) {
      chSysHalt();
    }
    else{
      if (memcmp(teststring, buf, sizeof(teststring)) != 0){
        chSysHalt();
      }
      else{
        chprintf(chp, "OK\r\n");
      }
    }

    chprintf(chp, "Umount filesystem... ");
    f_mount(0, NULL);
    chprintf(chp, "OK\r\n");

    chprintf(chp, "Disconnecting from SDIO...");
    chThdSleepMilliseconds(100);
    if (sdcDisconnect(&SDCD1))
      chSysHalt();
    chprintf(chp, " OK\r\n");
    chprintf(chp, "------------------------------------------------------\r\n");
    chprintf(chp, "All tests passed successfully.\r\n");
    chThdSleepMilliseconds(100);
  }
  else{
    chSysHalt();
  }
}

void cmd_debug(BaseSequentialStream *chp, int argc, char *argv[]) {
  FRESULT err;
  FIL FileObject;
  uint32_t bytes_written;
  int i;
  uint8_t teststring[] = {"65536 65535 65534 65533 65532\r\n"};
  
  (void)argv;
if (argc != 0) {
    chprintf(chp, "Usage: debug\r\n");
    return;
  }
  if (!sdcard_fs_ready) {
    chprintf(chp, "File System not mounted\r\n");
    return;
  }
  chprintf(chp, "Open test file (%u)...", chTimeNow());
  chThdSleepMilliseconds(100);
  err = f_open(&FileObject, "0:logtest.txt", FA_WRITE | FA_OPEN_ALWAYS);
  if (err != FR_OK) {
    chprintf(chp, "FS: File creation error\r\n");
  }
  chprintf(chp, "OK\r\n");
  chThdSleepMilliseconds(100);
  chprintf(chp, "Start writing test file (%u)...", chTimeNow());
  for(i = 0; i < 100; i++)
  {
    err = f_write(&FileObject, teststring, sizeof(teststring), (void *)&bytes_written);
    if (err != FR_OK) 
    {
      chprintf(chp, "FS: Write error\r\n");
      break;
    }
  }
  chprintf(chp, "OK (%u)\r\n", chTimeNow());
  err = f_close(&FileObject);
  if (err != FR_OK) {
    chprintf(chp, "FS: f_close() failed\r\n");
    return;
  }
}

