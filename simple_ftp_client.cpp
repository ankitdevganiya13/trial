/** @file simple_ftp_client.cpp
    @brief Function implimentation for ftp client library.

    This contains the implimentation for the ftp client library
    functionalities.

    @author Dhananjay Khairnar
*/
#include "simple_ftp_client.h"
// #include "SD_card_helper.h"
#include <SPIFFS.h>

WiFiClient fclient;
WiFiClient dataclient;
WiFiClient ftpClient;

String recvBuf;
char ftpRespDataBuf[100] = {0};

/** @brief flush receive buffer

    @return void
*/
void ftpflush()
{
  recvBuf = "";
  for (int i = 0 ; i < 50 ; i++)
  {
    while (ftpClient.available())
    {
      char ch = ftpClient.read();
      ch = '0';
    }
    delay(1);
  }
}

/** @brief establis connection to ftp server

    @return true if success / false if  failed
*/
bool connectToFTPServer()
{
  if (!ftpClient.connect(ftp_server.c_str(), ftpPort)) {
    if(dBug != false){
      FTPLogLn("FTP Server connection failed");
    }
    return false;
  }
  readFTPResponse();
  return true;
}

/** @brief This function read responce comming from ftp server

    read responses from ftp server and stores it in buffre \a recvBuf

    @return void
*/
void readFTPResponse()
{  
  String temp;
  char ch;
  for (int i = 0 ; i < 100 ; i++)
  {
    while (ftpClient.available())
    {
      ch = ftpClient.read();
      recvBuf += ch;
    }
    delay(1);
  }
}

/** @brief This function sends command to ftp server

    sends command to ftp server and get responce from ftp server,
     then it parse that respoce to get responce code.

    @param cmd ftp command

    @return ftp responce code
*/
uint32_t sendFTPCommand(String cmd)
{
  uint32_t ret = 0;
  if(dBug != false){
    FTPLog("< ");
    FTPLogLn(cmd);
  }
  ftpflush();
  ftpClient.flush();
  ftpClient.println(cmd);
  delay(DELAY_BETWEEN_COMMAND);
  volatile unsigned long t = millis();
  while ( ((millis() - t) < COMMAND_TIME_OUT) && (recvBuf.length() < 3))
    readFTPResponse();
  if(dBug != false){  
    FTPLog("> ");
    FTPLogLn(recvBuf); 
  }
  if (recvBuf.length())
    sscanf(recvBuf.c_str(), "%d ", &ret);
  char *ptr;
  ptr = (char*)recvBuf.c_str();
  while (*ptr)
  {
    if (isdigit (*ptr) ) {
      ret = strtoul (ptr, &ptr, 10);
      break;
    }
    ptr++;
  }

  return (uint32_t)ret;
}

/** @brief This function sends login sequence to ftp serce

    sends login sequeto ftp server and return status of login

    @param username username for login
    @param password password for login

    @return if login success then true / if login fails then false
*/
bool loginFTPServer(String username , String password)
{
  uint32_t ret = 0;
  uint8_t retryUser = LOGIN_RETRY;
  uint8_t retryPass = LOGIN_RETRY;
  delay(10);
  while (retryUser > 0)
  {
    ftpflush();
    if(dBug != false){
      Serial.println(ret);
    }
    ret = sendFTPCommand("USER " + username);
    if(dBug != false){
      Serial.println(ret);
    }
    if (USEROK == ret)
    {
      break;
    }
    delay(1000);
    retryUser--;
  }
  if(retryUser == 0){
    if(dBug != false){
      Serial.println("User Not Success");
    }
    return false;
  }
  while (retryPass > 0)
  {
    ftpflush();
    ret = sendFTPCommand("PASS " + password);
    if ( (LOGIN_SUCCESS == ret) | (ALREADY_LOGIN == ret) )
    {
      if(dBug != false){
        Serial.println("Login Success");
      }
      return true;
    }
    delay(1000);
    retryPass--;
  }
  if(dBug != false){
    Serial.println("Login Not Success");
  }
  return false;
}

/** @brief This function parse data port send by ftp server

    parse data port used for transfer data from ftp server and stores it in \a DataPort

    @return if parse success then true / if parse fails then false
*/
bool parseFTPDataPort()
{
  DataPort = 0;
  char *ptr;
  ptr = (char*)recvBuf.c_str();
  ptr += 3;
  while (*ptr)
  {
    if (isdigit (*ptr) ) {
      DataPort = strtoul (ptr, &ptr, 10);
    }
    ptr++;
  }
  if(dBug != false){
    FTPLogLn(String("Port Parsed ") + DataPort);
  }
  delay(200);
  if (DataPort > 0)
  {
    return true;
  }
  return false;
}

uint32_t parseFTPFileSize()
{
  ftpfileSize = 0;
  char *ptr;
  ptr = (char*)recvBuf.c_str();
  ptr += 3;
  while (*ptr)
  {
    if (isdigit (*ptr) ) {
      ftpfileSize = strtoul (ptr, &ptr, 10);
    }
    ptr++;
  }

  if(dBug != false){
    FTPLogLn(String("FTP File Size Parsed ") + ftpfileSize);
  }
  delay(200);
  if (ftpfileSize > 0)
  {
    return ftpfileSize;
  }
  return false;
}

/** @brief This function initialize file download sequence

    @return if success then true / if fails then false
*/
bool initFileDownloadSequence(String path , String filename)
{
  uint32_t ret = 0;
  // uint8_t retryCwd = LOGIN_RETRY;
  uint8_t retryEpsv = LOGIN_RETRY;
  uint8_t retryFSIZE = LOGIN_RETRY;
  delay(10);
  while (retryFSIZE > 0)
  {
    ftpflush();
    ret = sendFTPCommand("SIZE " + filename);
    if (FILESIZE == ret)
    {
      if(dBug != false){
        Serial.println("SIZE Command Success");
      }
      delay(10);
      parseFTPFileSize();
      break;
    }
    delay(1000);
    retryFSIZE--;
  }
  if(retryFSIZE == 0){
    if(dBug != false){
      Serial.println("SIZE Command Not Success");
    }
    return false;
  }

  while (retryEpsv > 0)
  {
    ftpflush();
    ret = sendFTPCommand("EPSV");
    if (EPSV_SUCCESS == ret)
    {
      if(dBug != false){
        Serial.println("EPSV Command Success");
      }
      return parseFTPDataPort();
    }
    delay(1000);
    retryEpsv--;
  }
  if(dBug != false){
    Serial.println("EPSV Command Not Success");
  }
  return false;
}

/** @brief This function download file from ftp server(Using socket api)

    function download file from ftp server. before calling this function you have to
    connect to ftp saerver then login using connectToFTPServer() and loginFTPServer() functions

    @param path path of file on ftp folder

    @return true if success / false if fails
*/
bool downloadFileFromFTP(String path, String filename)
{
  int socket_fd;
  struct sockaddr_in ra;
  File file;
  int recvDataCount = 0;
  uint8_t data_buffer[5*1024];
  if (!initFileDownloadSequence(path,filename))
  {
    if(dBug != false){
      Serial.println("Initialzation Not Success.");
    }
    return false;
  }
  socket_fd = socket(PF_INET, SOCK_STREAM, 0);
  if ( socket_fd < 0 )
  {
    printf("socket call failed");
    return false;
  }
  /* Receiver connects to server ip-address. */
  memset(&ra, 0, sizeof(struct sockaddr_in));
  ra.sin_family = AF_INET;
  ra.sin_addr.s_addr = inet_addr(ftp_server.c_str());
  ra.sin_port = htons(DataPort);
  if (connect(socket_fd, (const sockaddr*)&ra, sizeof(struct sockaddr_in)) < 0)
  {
    printf("connect failed \n");
    close(socket_fd);
    return false;
  }
  int recvCount = 0;
  if(ftpfileSize < SpiffsRSize){
    if (OPENING_FILE == sendFTPCommand("RETR " + filename))
    {
      SPIFFS.remove("/WiFi.bin");
      file = SPIFFS.open("/WiFi.bin", FILE_WRITE);
      if (!file) {
        if(dBug != false){
          FTPLogLn("Failed to open file for writing");
        }
        close(socket_fd);
        return false;
      }
    }
    else
    {
      close(socket_fd);
      return false;
    }

    if(dBug != false){
      FTPLogLn("Downloading File, Please Wait.");
    }
    uint32_t t = millis();
    while ( (recvDataCount = recv(socket_fd, data_buffer, sizeof(data_buffer), 0)) > 0)
    {
      file.write(data_buffer, recvDataCount);
      recvCount += recvDataCount;
    }
    uint32_t ttotal = (millis() - t);
    if(dBug != false){
      Serial.printf("Download Completed, Time required %d ms, File Size %d bytes\n\n" , ttotal , recvCount);
    }
    file.close();
    close(socket_fd);
    return true;
  }
  else{
    if(dBug != false){
      Serial.printf("Spiffs has low free space.\r\nFreeSpace: %d & FileSize: %d\r\n",SpiffsRSize,ftpfileSize);
    }
    return false;
  }
}