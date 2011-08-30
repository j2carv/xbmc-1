#pragma once
//#include "windows.h"

/*extern "C"
{
#include "cmyth/cmyth.h"
#include "refmem/refmem.h"
}*/

//#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "utils/StdString.h"
#include "libcmyth.h"

class MythChannel;
class MythRecorder;
typedef std::vector<MythChannel> MythChannelList;

class MythRecorder 
{
public:
  MythRecorder();
  MythRecorder(cmyth_recorder_t cmyth_recorder);
  bool SpawnLiveTV(MythChannel &channel);
  bool LiveTVChainUpdate(CStdString chainID);
  bool IsNull();
  bool IsRecording();
  bool CheckChannel(MythChannel &channel);
  bool SetChannel(MythChannel &channel);
  int ReadLiveTV(void* buffer,long long length);
private:
  class cmythRec;
  boost::shared_ptr<cmythRec> m_recorder_t;
  static void prog_update_callback(cmyth_proginfo_t prog);
};


class MythEventHandler 
{
public:
  MythEventHandler();
  MythEventHandler(CStdString,unsigned short port);
  void AddRecorder(MythRecorder &rec);
private:
  class ImpMythEventHandler;
  boost::shared_ptr<ImpMythEventHandler> m_imp;

};

typedef cmyth_program_t MythProgram;
/*class MythProgram
{
public:
  MythProgram();
  MythProgram(cmyth_program_t cmyth_program);
private:
  class cmythProgram;
  boost::shared_ptr<cmythProgram> m_program_t;
};*/

class MythDatabase
{
public:
  MythDatabase();
  MythDatabase(CStdString server,CStdString database,CStdString user,CStdString password);
  std::vector<MythChannel> ChannelList();
  std::vector<MythProgram> GetGuide(time_t starttime, time_t endtime);
private:
  class cmythDB;
  boost::shared_ptr<cmythDB> m_database_t;
};

class MythChannel
{
public:
  MythChannel();
  MythChannel(cmyth_channel_t cmyth_channel);
  int ID();
  int Number();
  CStdString Name();
  CStdString Icon();
  bool Visible();
private:
  class cmythChan;
  boost::shared_ptr<cmythChan> m_channel_t;
};


class MythConnection 
{
public:
  MythConnection();
  MythConnection(CStdString server,unsigned short port);
  MythRecorder GetFreeRecorder();
  MythEventHandler CreateEventHandler();
  bool IsConnected();
  CStdString GetServer();
  int GetProtocolVersion();
  bool GetDriveSpace(long long &total,long long &used);
private:
  class cmythConn;
  boost::shared_ptr<cmythConn> m_conn_t;
  CStdString m_server;
  unsigned short m_port;
};