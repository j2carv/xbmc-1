#include "fileOps.h"
#include "FileItem.h"
#include "Util.h"
#include "filesystem/File.h"
#include <stdio.h>
#include <map>
#include <ctime>
#include "libXBMC_addon.h"
#include "filesystem/SpecialProtocol.h"
#include "cppmyth/MythFile.h"
#include <pvr/recordings/PVRRecordings.h>


extern ADDON::CHelper_libXBMC_addon *XBMC;
using namespace ADDON;

fileOps::fileOps()
:myth_server(""),myth_port(0)
{
  
  baseLocalCachepath = "special://home/cache/";
  checkDirectory(baseLocalCachepath);
  isMyth = false;
  
}

fileOps::fileOps(MythConnection &mythConnection)
  :myth_server(""),myth_port(0),baseLocalCachepath(g_szUserPath.c_str()),mythConP(mythConnection),isMyth(true)
{
  baseLocalCachepath /= "cache";
  checkDirectory(baseLocalCachepath);
  XBMC->Log(LOG_DEBUG,"%s: mythConnection - Connection created!",__FUNCTION__);
}
 

fileOps::fileOps(CStdString mythServer,int mythPort)
:myth_server(mythServer),myth_port(mythPort)
{

  mythConP = MythConnection(mythServer,mythPort);
  if(!mythConP.IsConnected())
  {
    XBMC->QueueNotification(QUEUE_ERROR,"%s: mythConnection - Not Connected - %s:%i",
			    __FUNCTION__,mythServer.c_str(),mythPort);
    isMyth = false;
  }
  else {
    isMyth = true;
    XBMC->Log(LOG_DEBUG,"%s: mythConnection - Connection created!",__FUNCTION__);
  }
  
  baseLocalCachepath = "special://home/cache/";
  checkDirectory(baseLocalCachepath);
}

void fileOps::checkRecordings ( PVR_HANDLE handle )
{ 
  // Experimenting with updating videoinfotag
  PVR::CPVRRecordings *someRecordings = (PVR::CPVRRecordings*) handle->dataAddress;
  
  if ((someRecordings->size() > 0)  && (isMyth)) 
  {
    CFileItemList fileList;
    someRecordings->GetRecordings(&fileList);
    
    for (int i=0;i<fileList.Size();i++) 
    {
      
      PVR::CPVRRecording* pvrTag = fileList.Get(i)->GetPVRRecordingInfoTag();
      CStdString iconPath = getArtworkPath(pvrTag->m_strTitle.c_str(),FILE_OPS_GET_COVERART);
      if (!iconPath.IsEmpty()) 
      {
	someRecordings->UpdateEntry(*pvrTag);
      }
    }    
  }  
}

/*
* Function takes the title of a show, and a folder to search in
* returns a path to the artwork
*/
CStdString fileOps::getArtworkPath ( CStdString title, FILE_OPTIONS Get_What )
{
  CStdString retPath;
  if (title.IsEmpty()) 
    return retPath;
  
  CStdString awGroup;
  if (Get_What == FILE_OPS_GET_CHAN_ICONS)
    awGroup = "channels";
  else if (Get_What == FILE_OPS_GET_COVERART)
    awGroup = "coverart";
  else if (Get_What == FILE_OPS_GET_FANART)
    awGroup = "fanart";
  else
    return retPath;
  
  if (Get_What == FILE_OPS_GET_CHAN_ICONS) 
  {
    boost::filesystem::path someUrl(title.c_str());
    title = someUrl.filename().c_str();
  }
  else {
    int mrkrPos = title.find_first_of("::");
    if (mrkrPos<=0)
      mrkrPos=title.length();
    
    title = title.Left(mrkrPos);
  }
  
  XBMC->Log(LOG_DEBUG,"%s - ## - Checking for Artwork File - %s - in - %s - ##",
	    __FUNCTION__, title.c_str(), awGroup.c_str());
  
  retPath = checkFolderForTitle(title,awGroup);
  
  if (retPath.IsEmpty() && isMyth)
  {
    // If still no result check if we can sync with mythbackends storage group and try again
    if (!mythConP.IsConnected()) 
    {
      XBMC->Log(LOG_DEBUG,"%s - ###########Not connected to mythbackend#######", __FUNCTION__);
      return retPath;
    }
    
    if (Get_What == FILE_OPS_GET_CHAN_ICONS) 
    {
      GetFileFromBackend(title,"channels");
    }
    else
    {
      syncSGCache(awGroup); //Thread this function, so xbmc doesnt hang when getting recordings.
    } 
    retPath = checkFolderForTitle(title,awGroup);
  }
  
  return retPath;
}

CStdString fileOps::convSpecialPath (CStdString thePath, bool toRegular/*=true*/) {
  if (toRegular) // default
  {
    return CSpecialProtocol::TranslatePath(thePath);
  }
  else 
  {
    //TODO::converts Regular to special??
    //CUtil::TranslateSpecialSource(thePath);
  }
}

CStdString fileOps::GetFileFromBackend ( CStdString filenameToGet, CStdString fromStorageGroup )
{
  XBMC->Log(LOG_DEBUG,"%s - Getting File via Myth Protocol - %s",
	    __FUNCTION__,filenameToGet.c_str());
  
  if (filenameToGet.Left(1).compare("/") != 0) 
  {
    filenameToGet = "/" + filenameToGet;
  }
  
  MythFile theFile;
  
  if (fromStorageGroup.CompareNoCase("channels")==0) 
  {
    CStdString chanFilename = "/channels" + filenameToGet;
    CStdString chanSG = "c";
    theFile=mythConP.ConnectPath((char*)chanFilename.c_str(),(char*)chanSG.c_str());
  }
  else 
  {
    theFile=mythConP.ConnectPath((char*)filenameToGet.c_str(),(char*)fromStorageGroup.c_str());
  }

  if (theFile.IsNull()) 
  {
    return "";
  }
  int theFilesLength = theFile.Duration();
  if (theFilesLength <= 0) 
  {
    return "";
  }
  
  CStdString writeFilePath = baseLocalCachepath + fromStorageGroup + filenameToGet;
  checkDirectory(writeFilePath,true);
  
  XFILE::CFile writeFile;
  if (writeFile.OpenForWrite(writeFilePath))
  {
    //char* theFileBuff = new char[theFilesLength];
    long long  totalRead = 0;
    unsigned int buffersize = 4096;
    char* theFileTmpBuff = new char[buffersize];
    long long  readsize = 1024;
    while (totalRead < theFilesLength)
    {
      char* theFileTmpBuff = new char[theFilesLength];
      int readData = theFile.Read(theFileTmpBuff,theFilesLength-totalRead);
      if (readData <= 0)
      {
        break;
      }
      writeFile.Write((const void*)theFileTmpBuff,readData);
      /*char *filePntr = theFileBuff;
      if (totalRead > 0) {
        filePntr += (totalRead);
      }
      if (readData > (theFilesLength-totalRead)) {
        readData = (theFilesLength-totalRead);
      }
      memcpy((void*)filePntr,(void*)theFileTmpBuff,readData);
      */
      
      totalRead += readData;
    }
    writeFile.Close();
    if (totalRead < theFilesLength) 
    {
    XBMC->Log(LOG_DEBUG,"%s - Did not Read all data - %s - %d - %d",
	      __FUNCTION__,filenameToGet.c_str(),totalRead,theFilesLength);    
    }
    return writeFilePath;
  }
  else 
  {
    return "";
  }
  
}

void fileOps::syncSGCache ( CStdString awGroup )
{
  
  XBMC->Log(LOG_DEBUG,"%s - Syncing Storage Groups - %s",__FUNCTION__,awGroup.c_str());
  time_t curTime;
  time(&curTime);
  
  if (((int)curTime - lastSGupdate[awGroup]) < 30) { // Limit storage group updates to once every 30 seconds
    return;
  }
  
  lastSGupdate[awGroup] = (int)curTime;
  //Get Missing files
  std::vector<CStdString> filesToGet = missingSGFiles(mythConP.GetStorageGroupFileList(awGroup),LocalFilelist[awGroup]);
  
  
  for (unsigned int a=0;a<filesToGet.size();a++) 
  {
    XBMC->Log(LOG_DEBUG,"%s - Missing File - %s - FROM - %s",__FUNCTION__,filesToGet[a].c_str(),awGroup.c_str());
    GetFileFromBackend(filesToGet[a],awGroup);
  }
  
  if (filesToGet.size()>0) 
  {
    updateLocalFilesList(awGroup);
  }
  XBMC->Log(LOG_DEBUG,"%s - Sync Done - %s",__FUNCTION__,awGroup.c_str());
}

CStdString fileOps::checkFolderForTitle ( CStdString title, CStdString awGroup )
{
  
  CStdString retPath;
  std::vector<CStdString> fileList = LocalFilelist[awGroup];
  for (unsigned int curFl=0;curFl<fileList.size();curFl++) 
  {
    if (title.CompareNoCase(fileList[curFl].Left(title.length()).c_str()) == 0) 
    {
      //Found a Title match
      retPath = baseLocalCachepath + awGroup + "/" + fileList[curFl];
      break;
    }
  }
  if (retPath.IsEmpty())
  {
    updateLocalFilesList(awGroup);
    std::vector<CStdString> fileList = LocalFilelist[awGroup];
    for (unsigned int curFl=0;curFl<fileList.size();curFl++) 
    {
      if (title.CompareNoCase(fileList[curFl].Left(title.length()).c_str()) == 0) 
      {
        //Found a Title match
        retPath = baseLocalCachepath + awGroup + "/" + fileList[curFl];
        break;
      }
    }
  }
  
  return retPath;
}


bool fileOps::checkDirectory ( CStdString dirPath, bool hasFilename/* = false */)
{
  int pos = 0;
  int offset = 0;
  CStdString srchString = dirPath;
  while (pos >= 0) {
    pos = srchString.find_first_of("/",pos+1);
    if (pos < 0) {
      break;
    }
    if (pos <= baseLocalCachepath.length()) {
      continue;
    }
    CUtil::CreateDirectoryEx(srchString.Left(pos)); 
    if (offset > 20) {
      break;
    }
    offset++;
  }
  if (!hasFilename) {
    CUtil::CreateDirectoryEx(dirPath);
  }
  return true;
}

std::vector<CStdString> fileOps::getADirectoryList ( CStdString dirPath )
{
  CFileItemList items;
  CUtil::GetRecursiveListing(dirPath, items, "", true);
  std::vector<CStdString> retStr;
  for (int i = 0; i < items.Size(); ++i) 
  {
    retStr.push_back(items[i]->GetLabel());
  }
  
  return retStr;
}


bool fileOps::updateLocalFilesList (CStdString localFolder) 
{
  CStdString localPath = baseLocalCachepath + localFolder + "/";
  if ((CUtil::CreateDirectoryEx(baseLocalCachepath)) 
    && (CUtil::CreateDirectoryEx(localPath))) 
  {
    
    LocalFilelist[localFolder] = getADirectoryList(localPath);
    return true;
  }
  else 
  {
    return false;
  }
}

std::vector<CStdString> fileOps::missingSGFiles ( std::vector<CStdString> hayStack, std::vector<CStdString> needle )
{
  // Returns items in haystack that are not in needle
  
  std::vector<CStdString> missingFiles;
  
  for (unsigned int i = 0; i < hayStack.size();i++) 
  {
    CStdString remoteFile = hayStack[i].c_str();
    bool flMissing = true;
    for (unsigned int k = 0; k < needle.size();k++) 
    {
      CStdString localFile = needle[k].c_str();
      if (remoteFile.CompareNoCase(localFile.c_str()) == 0) 
      {
	flMissing = false;
	break;
      }
    }
    if (flMissing) 
    {
      missingFiles.push_back(remoteFile.c_str());
    }
  }
  return missingFiles;
}

void fileOps::saveSGList (std::vector<CStdString> sgFileList, CStdString sgToSaveAs) 
{
  
  SGFilelist[sgToSaveAs] = sgFileList;
  
}

bool fileOps::storeFileInSG ( char* saveBuffer, int buffLength, CStdString flSaveName, CStdString sgToSaveIn )
{
  CStdString theLocalPath;
  
  if (flSaveName.Left(1).compare("/") == 0) 
  {
    theLocalPath = baseLocalCachepath + sgToSaveIn + flSaveName;
  }
  else 
  {
    theLocalPath = baseLocalCachepath + sgToSaveIn + "/" + flSaveName;
  }
  return writeToFile(theLocalPath,saveBuffer,buffLength);
}

bool fileOps::writeToFile ( CStdString filePath, char* writeBuffer, int writeLength )
{
  
  XFILE::CFile file;
  if (file.OpenForWrite(filePath)) 
  {
    file.Write((const void*)writeBuffer,writeLength);
    file.Close();
    return true;
  }
  else 
  {
    return false;
  }
  
}
