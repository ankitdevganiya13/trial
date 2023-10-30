/** @file simple_ftp_client.h
    @brief Function prototypes for ftp client library.

    This contains the prototypes for the ftp client library
    and eventually any macros, constants,
    or global variables you will need.

    @author Dhananjay Khairnar
*/

#ifndef __SIMPLE_FTP_CLIENT_H
#define __SIMPLE_FTP_CLIENT_H
#include <Arduino.h>
#include <WiFi.h>
#include <lwip/sockets.h>

/**Comment this macro to disabl e ftp logs */
#define FTP_Log 

#ifdef FTP_Log
#define FTPLog(...) Serial.print(__VA_ARGS__)
#define FTPLogLn(...) Serial.println(__VA_ARGS__)
#define FTPLogF(...) Serial.printf(__VA_ARGS__)
#else
#define FTPLog(...)
#define FTPLogLn(...)
#define FTPLogF(...)
#endif

#define DELAY_BETWEEN_COMMAND 500 //ms
#define COMMAND_TIME_OUT 2000 //ms
#define LOGIN_RETRY 4

/** FTP response code*/
typedef enum EN_FTPResCode {
  OPENING_FILE = 150,
  LOGIN_SUCCESS = 230,
  ALREADY_LOGIN = 503,
  CWD_SUCCESS = 250,
  EPSV_SUCCESS = 229,
  USEROK = 331,
  FILESIZE = 213,
  FILE_NOT_FOUND = 550
} EN_FTPResCode;

/*
   Variable expose form simple_ftp_client.cpp
*/
/** This variable maintain ftp server ip address eg. "192.168.1.2"*/
extern String ftp_server;
extern bool dBug;
/** This variable maintain ftp server port default 21*/
extern unsigned int ftpPort;
extern uint32_t ftpfileSize;
extern uint32_t SpiffsRSize;
/** This variable maintain ftp data channel port can be parse by function \a parseFTPDataPort()*/
extern unsigned int DataPort;


/*
   Function prototypes for simple_ftp_client.cpp
*/
bool connectToFTPServer();
uint32_t sendFTPCommand(String cmd);
bool loginFTPServer(String username , String password);
void readFTPResponse();
bool parseFTPDataPort();
uint32_t parseFTPFileSize();
bool initFileDownloadSequence(String path);
bool downloadFileFromFTP(String path, String filename);
bool getFile(String path);
#endif
