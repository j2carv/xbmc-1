#include "MythRecorder.h"
#include "MythProgramInfo.h"
#include "MythChannel.h"
#include "client.h"

using namespace ADDON;

/*
*								Myth Recorder
*/


MythRecorder::MythRecorder():
m_recorder_t(),livechainupdated()
{
}

MythRecorder::MythRecorder(cmyth_recorder_t cmyth_recorder):
m_recorder_t(new MythPointerThreadSafe<cmyth_recorder_t>()),livechainupdated(new int(0))
{
  *m_recorder_t=cmyth_recorder;
}

bool MythRecorder::SpawnLiveTV(MythChannel &channel)
{
  char* pErr=NULL;
  CStdString channelNum;
  channelNum.Format("%i",channel.Number());
  m_recorder_t->Lock();
  //check channel
  *livechainupdated=0;
  *m_recorder_t=(CMYTH->SpawnLiveTv(*m_recorder_t,64*1024, 16*1024,MythRecorder::prog_update_callback,&pErr,channelNum.GetBuffer()));
  int i=20;
  while(*livechainupdated==0&&i--!=0)
  {
    m_recorder_t->Unlock();
    cSleep(100);
    m_recorder_t->Lock();
  }
  m_recorder_t->Unlock();
  ASSERT(*m_recorder_t);
  
  if(pErr)
    XBMC->Log(LOG_ERROR,"%s - %s",__FUNCTION__,pErr);
  return pErr==NULL;
}

bool MythRecorder::LiveTVChainUpdate(CStdString chainID)
{
  char* buffer=strdup(chainID.c_str());
  m_recorder_t->Lock();
  bool retval=CMYTH->LivetvChainUpdate(*m_recorder_t,buffer,16*1024)==0;
  if(!retval)
    XBMC->Log(LOG_ERROR,"LiveTVChainUpdate failed on chainID: %s",buffer);
  *livechainupdated=1;
  m_recorder_t->Unlock();
  free(buffer);
  return retval;
}

void MythRecorder::prog_update_callback(cmyth_proginfo_t prog)
{
  XBMC->Log(LOG_DEBUG,"prog_update_callback");

}


bool MythRecorder::IsNull()
{
  if(m_recorder_t==NULL)
    return true;
  return *m_recorder_t==NULL;
}



bool MythRecorder::IsRecording()
{
  m_recorder_t->Lock();
  bool retval=CMYTH->RecorderIsRecording(*m_recorder_t)==1;
  m_recorder_t->Unlock();
  return retval;
}

bool MythRecorder::CheckChannel(MythChannel &channel)
{
  m_recorder_t->Lock();
  CStdString channelNum;
  channelNum.Format("%i",channel.Number());
  bool retval=CMYTH->RecorderCheckChannel(*m_recorder_t,channelNum.GetBuffer())==0;
  m_recorder_t->Unlock();
  return retval;
}

bool MythRecorder::SetChannel(MythChannel &channel)
{
  m_recorder_t->Lock();
  if(!IsRecording())
  {
    XBMC->Log(LOG_ERROR,"%s: Recorder %i is not recording",__FUNCTION__,ID(),channel.Name().c_str());
    m_recorder_t->Unlock();
    return false;
  }
  CStdString channelNum;
  channelNum.Format("%i",channel.Number());
  if(CMYTH->RecorderPause(*m_recorder_t)!=0)
  {
    XBMC->Log(LOG_ERROR,"%s: Failed to pause recorder %i",__FUNCTION__,ID());
    m_recorder_t->Unlock();
    return false;
  }
  if(!CheckChannel(channel))
  {
    XBMC->Log(LOG_ERROR,"%s: Recorder %i doesn't provide channel %s",__FUNCTION__,ID(),channel.Name().c_str());
    m_recorder_t->Unlock();
    return false;
  }
  if(CMYTH->RecorderSetChannel(*m_recorder_t,channelNum.GetBuffer())!=0)
  {
    XBMC->Log(LOG_ERROR,"%s: Failed to change recorder %i to channel %s",__FUNCTION__,ID(),channel.Name().c_str());
    m_recorder_t->Unlock();
    return false;
  }
  if(CMYTH->LivetvChainSwitchLast(*m_recorder_t)!=1)
  {
    XBMC->Log(LOG_ERROR,"%s: Failed to switch chain for recorder %i",__FUNCTION__,ID(),channel.Name().c_str());
    m_recorder_t->Unlock();
    return false;
  }
  *livechainupdated=0;
  int i=20;
  while(*livechainupdated==0&&i--!=0)
  {
    m_recorder_t->Unlock();
    cSleep(100);
    m_recorder_t->Lock();
  }

  m_recorder_t->Unlock();
  for(int i=0;i<20;i++)
  {
    if(!IsRecording())
      cSleep(1);
    else
      break;
  }

  return true;
}

int MythRecorder::ReadLiveTV(void* buffer,long long length)
{
  m_recorder_t->Lock();
  int bytesRead=CMYTH->LivetvRead(*m_recorder_t,static_cast<char*>(buffer),length);
  m_recorder_t->Unlock();
  return bytesRead;
}

MythProgramInfo MythRecorder::GetCurrentProgram()
{
  m_recorder_t->Lock();
  MythProgramInfo retval=CMYTH->RecorderGetCurProginfo(*m_recorder_t);
  m_recorder_t->Unlock();
  return retval;
}

long long MythRecorder::LiveTVSeek(long long offset, int whence)
{
  m_recorder_t->Lock();
  long long retval = CMYTH->LivetvSeek(*m_recorder_t,offset,whence);
  m_recorder_t->Unlock();
  return retval;
}

long long MythRecorder::LiveTVDuration()
{
  m_recorder_t->Lock();
  long long retval = CMYTH->LivetvChainDuration(*m_recorder_t);
  m_recorder_t->Unlock();
  return retval;
}

int MythRecorder::ID()
{
  return CMYTH->RecorderGetRecorderId(*m_recorder_t);
}

 bool  MythRecorder::Stop()
 {
   return CMYTH->RecorderStopLivetv(*m_recorder_t)==0;
 }
