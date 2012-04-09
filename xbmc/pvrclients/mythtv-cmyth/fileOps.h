#ifndef __FILEOPS_H
#define __FILEOPS_H
//TODO merge into MythConnection ??

#include "utils/StdString.h"
#include <vector>
#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

#include <map>

class MythConnection;

typedef enum
  {
    FILE_OPS_GET_COVERART   = 0,
    FILE_OPS_GET_FANART     = 1, /*!< @brief the timer is scheduled for recording */
    FILE_OPS_GET_CHAN_ICONS = 2, /*!< @brief the timer is currently recordings */
  } FILE_OPTIONS;
  
class fileOps
{
public:
  fileOps();
  fileOps(CStdString mythServer,int mythPort);
  fileOps(MythConnection &mythConnection);
  
  bool checkDirectory(boost::filesystem::path dirPath, bool hasFilename = false);
  std::vector< CStdString > getADirectoryList(boost::filesystem::path dirPath);
  bool writeToFile( boost::filesystem::path filePath, char* writeBuffer, int writeLength );
  void saveSGList (std::vector<CStdString> sgFileList, CStdString sgToSaveAs);
  bool updateLocalFilesList (CStdString localFolder);
  bool storeFileInSG( char* saveBuffer, int buffLength, CStdString flSaveName, CStdString sgToSaveIn );
  std::vector<CStdString> missingSGFiles ( std::vector<CStdString> hayStack, std::vector<CStdString> needle );
  CStdString getArtworkPath(CStdString title,FILE_OPTIONS Get_What);
  CStdString checkFolderForTitle(CStdString title,CStdString awGroup);
  void syncSGCache(CStdString awGroup);
  CStdString GetFileFromBackend ( CStdString filenameToGet, CStdString fromStorageGroup );
  void checkRecordings (PVR_HANDLE handle);
  
  
private:
  CStdString myth_server;
  int myth_port;
  
  std::map< CStdString, std::vector< CStdString > > SGFilelist;
  std::map< CStdString, std::vector< CStdString > > LocalFilelist;
  boost::filesystem::path baseLocalCachepath;
  std::map< CStdString, int > lastSGupdate;
  MythConnection mythConP;
  bool isMyth;
};

#endif