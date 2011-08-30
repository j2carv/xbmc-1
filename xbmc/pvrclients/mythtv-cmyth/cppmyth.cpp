#include "cppmyth.h"
#include <sstream>
#include "client.h"
#include "thread.h"
/*
*								MythEventHandler
*/

class MythEventHandler::ImpMythEventHandler : public cThread
{
public:
  ImpMythEventHandler(CStdString server,unsigned short port);
  MythRecorder m_rec;
  virtual void Action(void);	
  cmyth_conn_t m_conn_t;
  virtual ~ImpMythEventHandler();
  };

MythEventHandler::ImpMythEventHandler::ImpMythEventHandler(CStdString server,unsigned short port)
:m_rec(0),m_conn_t(0),cThread("MythEventHandler")
  {
    char *cserver=_strdup(server.c_str());
    cmyth_conn_t connection=CMYTH->ConnConnectEvent(cserver,port,16*1024, 4096);
    free(cserver);
    m_conn_t=connection; 
  }


  MythEventHandler::ImpMythEventHandler::~ImpMythEventHandler()
  {
    Cancel(30);
    CMYTH->RefRelease(m_conn_t);
    m_conn_t=0;
  }

MythEventHandler::MythEventHandler(CStdString server,unsigned short port)
  :m_imp(new ImpMythEventHandler(server,port))
{
  m_imp->Start();
}

MythEventHandler::MythEventHandler()
  :m_imp()
{
}

void MythEventHandler::AddRecorder(MythRecorder &rec)
{
  
  m_imp->m_rec=rec;
}




void MythEventHandler::ImpMythEventHandler::Action(void)
{
  const char* events[]={	"CMYTH_EVENT_UNKNOWN",\
    "CMYTH_EVENT_CLOSE",\
    "CMYTH_EVENT_RECORDING_LIST_CHANGE",\
    "CMYTH_EVENT_RECORDING_LIST_CHANGE_ADD",\
    "CMYTH_EVENT_RECORDING_LIST_CHANGE_UPDATE",\
    "CMYTH_EVENT_RECORDING_LIST_CHANGE_DELETE",\
    "CMYTH_EVENT_SCHEDULE_CHANGE",\
    "CMYTH_EVENT_DONE_RECORDING",\
    "CMYTH_EVENT_QUIT_LIVETV",\
    "CMYTH_EVENT_WATCH_LIVETV",\
    "CMYTH_EVENT_LIVETV_CHAIN_UPDATE",\
    "CMYTH_EVENT_SIGNAL",\
    "CMYTH_EVENT_ASK_RECORDING",\
    "CMYTH_EVENT_SYSTEM_EVENT",\
    "CMYTH_EVENT_UPDATE_FILE_SIZE",\
    "CMYTH_EVENT_GENERATED_PIXMAP",\
    "CMYTH_EVENT_CLEAR_SETTINGS_CACHE"};
  cmyth_event_t myth_event;
  char databuf[2049];
  databuf[0]=0;
  timeval t;
  t.tv_sec=5;
  t.tv_usec=0;

  while(Running())
  {
    myth_event=CMYTH->EventGet(m_conn_t,databuf,2048);
    std::cout<<"EVENT ID: "<<events[myth_event]<<" EVENT databuf:"<<databuf<<std::endl;
    if(myth_event==CMYTH_EVENT_LIVETV_CHAIN_UPDATE&&!m_rec.IsNull())
    {

      std::cout<<"CHAIN_UPDATE: "<<m_rec.LiveTVChainUpdate(CStdString(databuf))<<std::endl;

    }
    databuf[0]=0;

  }
}



/*
*								MythDatabase
*/
class MythDatabase::cmythDB
{
public:
  cmyth_database_t get()
  {
    return m_database_t;
  }
  void set(cmyth_database_t db)
  {
    m_database_t=db;
  }
  ~cmythDB()
  {
    CMYTH->RefRelease(m_database_t);
    m_database_t=0;
  }
  cmythDB()
  {
    m_database_t=0;
  }
private:
  cmyth_database_t m_database_t;
};

MythDatabase::MythDatabase()
  :m_database_t()
{
}


MythDatabase::MythDatabase(CStdString server,CStdString database,CStdString user,CStdString password):
m_database_t(new cmythDB())
{
  char *cserver=_strdup(server.c_str());
  char *cdatabase=_strdup(database.c_str());
  char *cuser=_strdup(user.c_str());
  char *cpassword=_strdup(password.c_str());

  m_database_t->set(CMYTH->DatabaseInit(cserver,cdatabase,cuser,cpassword));
  free(cserver);
  free(cdatabase);
  free(cuser);
  free(cpassword);
}

std::vector<MythChannel> MythDatabase::ChannelList()
{
  std::vector<MythChannel> retval;
  cmyth_chanlist_t cChannels=CMYTH->MysqlGetChanlist(m_database_t->get());
  int nChannels=CMYTH->ChanlistGetCount(cChannels);
  for(int i=0;i<nChannels;i++)
    retval.push_back(MythChannel(CMYTH->ChanlistGetItem(cChannels,i)));
  CMYTH->RefRelease(cChannels);
  return retval;
}

std::vector<MythProgram> MythDatabase::GetGuide(time_t starttime, time_t endtime)
{
  MythProgram *programs;
  int len=CMYTH->MysqlGetGuide(m_database_t->get(),&programs,starttime,endtime);
  std::vector<MythProgram> retval(programs,programs+len);
  CMYTH->RefRelease(programs);
  return retval;
}
/*
*								MythChannel
*/
class MythChannel::cmythChan {
public:
  cmyth_channel_t get()
  {
    return m_channel_t;
  }
  void set(cmyth_channel_t channel)
  {
    m_channel_t=channel;
  }
  ~cmythChan()
  {
    CMYTH->RefRelease(m_channel_t);
    m_channel_t=0;
  }
  cmythChan()
  {
    m_channel_t=0;
  }
private:
  cmyth_channel_t m_channel_t;
};

MythChannel::MythChannel(cmyth_channel_t cmyth_channel)
  : m_channel_t(new cmythChan())
{
  m_channel_t->set(cmyth_channel);
}


MythChannel::MythChannel()
  : m_channel_t()
{
}

int  MythChannel::ID()
{
  return CMYTH->ChannelChanid(m_channel_t->get());
}

int  MythChannel::Number()
{
  return CMYTH->ChannelChannum(m_channel_t->get());
}

CStdString  MythChannel::Name()
{
  char* cChan=CMYTH->ChannelName(m_channel_t->get());
  CStdString retval(cChan);
  CMYTH->RefRelease(cChan);
  return retval;
}

CStdString  MythChannel::Icon()
{
  char* cIcon=CMYTH->ChannelIcon(m_channel_t->get());
  CStdString retval(cIcon);
  CMYTH->RefRelease(cIcon);
  return retval;
}

bool  MythChannel::Visible()
{
  return CMYTH->ChannelVisible(m_channel_t->get())>0;
}

/*   
*								MythConnection
*/
class MythConnection::cmythConn {
public:
  cmyth_conn_t get()
  {
    return(m_conn_t);
  }

  void set(cmyth_conn_t conn)
  {
    CMYTH->RefRelease(m_conn_t);
    m_conn_t=conn;
  }
  ~cmythConn()
  {
    CMYTH->RefRelease(m_conn_t);
    m_conn_t=0;
  }
  cmythConn():m_conn_t(0){}
private:
  //CRITICAL_SECTION  m_cs;
  cmyth_conn_t m_conn_t;
};

MythConnection::MythConnection():
m_conn_t(),m_server(""),m_port(0)
{  
}


MythConnection::MythConnection(CStdString server,unsigned short port):
m_conn_t(new MythConnection::cmythConn()),m_server(server),m_port(port)
{
  char *cserver=_strdup(server.c_str());
  cmyth_conn_t connection=CMYTH->ConnConnectCtrl(cserver,port,16*1024, 4096);
  free(cserver);
  m_conn_t->set(connection);
  
}

bool MythConnection::IsConnected()
{
  return m_conn_t->get()!=0;
}

MythRecorder MythConnection::GetFreeRecorder()
{
  return MythRecorder(CMYTH->ConnGetFreeRecorder(m_conn_t->get()));
}

MythEventHandler MythConnection::CreateEventHandler()
{
  return MythEventHandler(m_server,m_port);
}

CStdString MythConnection::GetServer()
{
  return m_server;
}

int MythConnection::GetProtocolVersion()
{
  return CMYTH->ConnGetProtocolVersion(m_conn_t->get());
}

bool MythConnection::GetDriveSpace(long long &total,long long &used)
{
  return CMYTH->ConnGetFreespace(m_conn_t->get(),&total,&used)==0;
}
/*
*								Myth Recorder
*/
class MythRecorder::cmythRec : public cMutex {
public:
  cmyth_recorder_t get()
  {
    return m_recorder_t;
  }
  void set(cmyth_recorder_t rec)
  {
    m_recorder_t=rec;
  }
  ~cmythRec()
  {
    std::cout<<"DELETING cmythRec: "<<this<<std::endl;
    CMYTH->RefRelease(m_recorder_t);
    m_recorder_t=0; 
  }
  cmythRec():m_recorder_t(0),cMutex()
  {
    std::cout<<"CREATING cmythRec: "<<this<<std::endl;
    
  }
private:
  cmyth_recorder_t m_recorder_t;
};

MythRecorder::MythRecorder():
m_recorder_t()
{
}

MythRecorder::MythRecorder(cmyth_recorder_t cmyth_recorder):
m_recorder_t(new cmythRec())
{
  m_recorder_t->set(cmyth_recorder);
}

bool MythRecorder::SpawnLiveTV(MythChannel &channel)
{
  char* pErr=NULL;
  CStdString channelNum;
  channelNum.Format("%i",channel.Number());
  m_recorder_t->Lock();
  m_recorder_t->set(CMYTH->SpawnLiveTv(m_recorder_t->get(),16*1024, 4096,MythRecorder::prog_update_callback,&pErr,channelNum.GetBuffer()));
  m_recorder_t->Unlock();
  if(pErr)
    std::cout<<__FUNCTION__<<pErr<<std::endl;
  return pErr==NULL;
}

bool MythRecorder::LiveTVChainUpdate(CStdString chainID)
{
  char* buffer=_strdup(chainID.c_str());
  m_recorder_t->Lock();
  bool retval=CMYTH->LivetvChainUpdate(m_recorder_t->get(),buffer,4096)!=0;
  m_recorder_t->Unlock();
  free(buffer);
  return retval;
}

void MythRecorder::prog_update_callback(cmyth_proginfo_t prog)
{
  std::cout<< "prog_update_callback";

}


bool MythRecorder::IsNull()
{
  return m_recorder_t->get()==NULL;
}



bool MythRecorder::IsRecording()
{
  m_recorder_t->Lock();
  bool retval=CMYTH->RecorderIsRecording(m_recorder_t->get())==1;
  m_recorder_t->Unlock();
  return retval;
}

bool MythRecorder::CheckChannel(MythChannel &channel)
{
  m_recorder_t->Lock();
  CStdString channelNum;
  channelNum.Format("%i",channel.Number());
  bool retval=CMYTH->RecorderCheckChannel(m_recorder_t->get(),channelNum.GetBuffer())==1;
  m_recorder_t->Unlock();
  return retval;
}

bool MythRecorder::SetChannel(MythChannel &channel)
{
  m_recorder_t->Lock();
  bool retval=CheckChannel(channel);
  if(!retval)
    return retval;
  CStdString channelNum;
  channelNum.Format("%i",channel.Number());
  retval=CMYTH->RecorderSetChannel(m_recorder_t->get(),channelNum.GetBuffer())==0;
  m_recorder_t->Unlock();
  return retval;
}

int MythRecorder::ReadLiveTV(void* buffer,long long length)
{
  m_recorder_t->Lock();
  int bytesRead=CMYTH->LivetvRead(m_recorder_t->get(),static_cast<char*>(buffer),length);
  m_recorder_t->Unlock();
  return bytesRead;
}

/*
 *          MythProgram
 */

/*
class MythProgram::cmythProgram
{
public:
  cmyth_program_t get()
  {
    return m_program_t;
  }
  void set(cmyth_program_t channel)
  {
    m_program_t=channel;
  }
  ~cmythProgram()
  {
    CMYTH->RefRelease(m_program_t);
    m_program_t=0;
  }
  cmythProgram()
  {
    m_program_t=0;
  }
private:
  cmyth_program_t m_program_t;

};

MythProgram::MythProgram():
m_program_t()
{
}


MythProgram::MythProgram(cmyth_program_t cmyth_program):
m_program_t(new cmythProgram(cmyth_program))
{
}
*/