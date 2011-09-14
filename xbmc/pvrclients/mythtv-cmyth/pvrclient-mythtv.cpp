#include "pvrclient-mythtv.h"
#include "client.h"


PVRClientMythTV::PVRClientMythTV()
  :m_con(),m_eventHandler(),m_db(),m_protocolVersion(""),m_connectionString(""),m_EPGstart(0),m_EPGend(0)
{
}

void Log(int l,char* msg)
{
   
  if(msg&&l!=CMYTH_DBG_NONE)
  {
    bool doLog=false;
    addon_log_t loglevel;
    switch( l)
    {
      case CMYTH_DBG_ERROR:
        loglevel=LOG_ERROR;
        doLog=true;
        break;
      case CMYTH_DBG_WARN:
      case CMYTH_DBG_INFO:
        loglevel=LOG_INFO;
        break;
      case CMYTH_DBG_DETAIL:
      case CMYTH_DBG_DEBUG:
      case CMYTH_DBG_PROTO:
      case CMYTH_DBG_ALL: 
        loglevel=LOG_DEBUG;
        break;
    }    
    if(XBMC&&doLog)
      XBMC->Log(loglevel,"LibCMyth: %s",  msg);
  }
}

bool PVRClientMythTV::Connect()
{
  CMYTH->DbgAll();
  CMYTH->SetDbgMsgcallback(Log);
  m_con=MythConnection(g_szHostname,g_iMythPort);
  if(!m_con.IsConnected())
    return false;
  m_eventHandler=m_con.CreateEventHandler();
  m_protocolVersion.Format("%i",m_con.GetProtocolVersion());
  m_connectionString.Format("%s:%i",g_szHostname,g_iMythPort);
  m_db=MythDatabase(g_szHostname,g_szMythDBname,g_szMythDBuser,g_szMythDBpassword);
  m_channels=m_db.ChannelList();
  for(unsigned int i=0;i<m_channels.size();i++)
    m_channelsMap.insert(std::pair<int,int>(m_channels[i].ID(),i));
  return true;
}

const char* PVRClientMythTV::GetBackendName()
{
  return m_con.GetServer();
}


const char * PVRClientMythTV::GetBackendVersion()
{
  return m_protocolVersion;
}

const char * PVRClientMythTV::GetConnectionString()
{
  return m_connectionString;
}

bool PVRClientMythTV::GetDriveSpace(long long *iTotal, long long *iUsed)
{
  return m_con.GetDriveSpace(*iTotal,*iUsed);
}

PVR_ERROR PVRClientMythTV::GetEPGForChannel(PVR_HANDLE handle, const PVR_CHANNEL &channel, time_t iStart, time_t iEnd)
{
  if(iStart!=m_EPGstart&&iEnd!=m_EPGend)
  {
    m_EPG=m_db.GetGuide(iStart,iEnd);
    m_EPGstart=iStart;
    m_EPGend=iEnd;
  }
  for(std::vector<MythProgram>::iterator it=m_EPG.begin();it!=m_EPG.end();it++)
  {
    if(it->chanid==channel.iUniqueId)
    {
      EPG_TAG tag;
      tag.endTime=it->endtime;
      tag.iChannelNumber=it->channum;
      tag.startTime=it->starttime;
      tag.strTitle=it->title;
      tag.strPlot= it->description;
      unsigned int seriesid=atoi(it->seriesid);
      if(seriesid!=0)
        tag.iUniqueBroadcastId=atoi(it->seriesid);
      else
        tag.iUniqueBroadcastId=(tag.startTime<<16)+(tag.iChannelNumber&0xffff);
      
      //unimplemented
      tag.strEpisodeName="";
      tag.strGenreDescription="";
      tag.strIconPath="";
      tag.strPlotOutline="";
      tag.bNotify=false;
      tag.firstAired=0;
      tag.iEpisodeNumber=0;
      tag.iEpisodePartNumber=0;
      tag.iGenreSubType=0;
      tag.iGenreType=0;
      tag.iParentalRating=0;
      tag.iSeriesNumber=0;
      tag.iStarRating=0;
      
      
      PVR->TransferEpgEntry(handle,&tag);
    }
   }
   return PVR_ERROR_NO_ERROR;
  }


int PVRClientMythTV::GetNumChannels()
{
  return m_channels.size();
}

//bool isRadio(MythChannel v){return v.IsRadio();}

PVR_ERROR PVRClientMythTV::GetChannels(PVR_HANDLE handle, bool bRadio)
{
  for (std::vector<MythChannel>::iterator it = m_channels.begin(); it != m_channels.end(); it++)
  {
    if (it->IsRadio()==bRadio)
    {
      PVR_CHANNEL tag;
      tag.bIsHidden=!it->Visible();
      tag.bIsRadio=it->IsRadio();
      tag.iUniqueId=it->ID();
      tag.iChannelNumber=it->ID(); //Swap ID and Number?
      CStdString chanName= it->Name();
      tag.strChannelName = chanName;
      CStdString icon = it->Icon();
      tag.strIconPath = icon;

      //Unimplemented
      tag.strStreamURL="";
      tag.strInputFormat="";
      tag.iEncryptionSystem=0;
      

      PVR->TransferChannelEntry(handle,&tag);
    }
  }
  return PVR_ERROR_NO_ERROR;
}

int PVRClientMythTV::GetRecordingsAmount(void)
{
  m_recordings=m_con.GetRecordedPrograms();
  return m_recordings.size();
}


PVR_ERROR PVRClientMythTV::GetRecordings(PVR_HANDLE handle)
{
  if(m_recordings.size()==0)
    m_recordings=m_con.GetRecordedPrograms();
for (boost::unordered_map<CStdString, MythProgramInfo>::iterator it = m_recordings.begin(); it != m_recordings.end(); it++)
  {
    PVR_RECORDING tag;
    tag.iDuration=it->second.Duration();
    tag.recordingTime=it->second.RecStart();
    CStdString chanName=it->second.ChannelName();
    CStdString plot=it->second.Description();
    CStdString path=it->second.Path();
    CStdString title=it->second.Title();

    tag.strChannelName=chanName;
    tag.strPlot=plot;
    /*char id[sizeof(long long)+1];
    id[sizeof(long long)]=0;
    *reinterpret_cast<long long*>(id)=it->second.uid();*/
    CStdString id=it->second.Path();
    tag.strRecordingId=id;
    CStdString group=it->second.RecordingGroup();
    tag.strDirectory=group=="Default"?"":group;
    tag.strTitle=title;


    //TODO: tag.iGenreType=it->Category();
    //Unimplemented
    tag.iGenreSubType=0;
    tag.iGenreType=0;
    tag.iLifetime=0;
    tag.iPriority=0;
    //tag.recordingTime=0;
    tag.strPlotOutline="";
    tag.strStreamURL="";
    
    
    
    PVR->TransferRecordingEntry(handle,&tag);
  }
  return PVR_ERROR_NO_ERROR;
}

 PVR_ERROR PVRClientMythTV::DeleteRecording(const PVR_RECORDING &recording)
 {
   
   //long long uid=*reinterpret_cast<const long long*>(recording.strRecordingId);
   CStdString id=recording.strRecordingId;
   bool ret = m_con.DeleteRecording(m_recordings[id]);
   if(ret && m_recordings.erase(recording.strRecordingId))
   {
     PVR->TriggerRecordingUpdate();
     return PVR_ERROR_NO_ERROR;

   }
   else
     return PVR_ERROR_NOT_DELETED;
 }

 int PVRClientMythTV::GetTimersAmount(void)
 {
  std::vector<MythTimer> m_timers=m_db.GetTimers();
  return m_timers.size();
 }

  PVR_ERROR PVRClientMythTV::GetTimers(PVR_HANDLE handle)
  {
    std::vector<MythTimer> m_timers=m_db.GetTimers();
    for (std::vector<MythTimer>::iterator it = m_timers.begin(); it != m_timers.end(); it++)
  {
    PVR_TIMER tag;
    tag.endTime=it->EndTime();
    tag.iClientChannelUid=it->ChanID();
    tag.iClientIndex=it->RecordID();
    tag.startTime=it->StartTime();
    CStdString title=it->Title();
    tag.strTitle=title;
    CStdString summary=it->Description(); 
    tag.strSummary=summary;
    tag.state=PVR_TIMER_STATE_SCHEDULED;
    
    //Unimplemented
    tag.bIsRepeating=false;
    tag.firstDay=0;
    tag.iEpgUid=0;
    tag.iGenreSubType=0;
    tag.iGenreType=0;
    tag.iLifetime=0;
    tag.iMarginEnd=0;
    tag.iMarginStart=0;
    tag.iPriority=0;
    tag.iWeekdays=0;
    tag.strDirectory="";
    
    PVR->TransferTimerEntry(handle,&tag);
  }
  return PVR_ERROR_NO_ERROR;
}

  PVR_ERROR PVRClientMythTV::AddTimer(const PVR_TIMER &timer)
  {

    int id=m_db.AddTimer(timer.iClientChannelUid,timer.strSummary,timer.startTime,timer.endTime,timer.strTitle);
    if(id<0)
      return PVR_ERROR_NOT_POSSIBLE;
    if(!m_con.UpdateSchedules(id))
      return PVR_ERROR_NOT_POSSIBLE;
    PVR->TriggerTimerUpdate();
    return PVR_ERROR_NO_ERROR;
  }

  PVR_ERROR PVRClientMythTV::DeleteTimer(const PVR_TIMER &timer, bool bForceDelete)
  {
    if(!m_db.DeleteTimer(timer.iClientIndex))
      return PVR_ERROR_NOT_POSSIBLE;
    m_con.UpdateSchedules(-1);
    PVR->TriggerTimerUpdate();
    return PVR_ERROR_NO_ERROR;
  }
  
  PVR_ERROR PVRClientMythTV::UpdateTimer(const PVR_TIMER &timer)
  {
    if(!m_db.UpdateTimer(timer.iClientIndex,timer.iClientChannelUid,timer.strSummary,timer.startTime,timer.endTime,timer.strTitle))
       return PVR_ERROR_NOT_POSSIBLE;
    m_con.UpdateSchedules(timer.iClientIndex);
    return PVR_ERROR_NO_ERROR;
  }


  bool PVRClientMythTV::OpenLiveStream(const PVR_CHANNEL &channel)
  {
    if(m_rec.IsNull())
    {
      m_rec=m_con.GetFreeRecorder();
      XBMC->Log(LOG_DEBUG,"%s: Opening new recorder %i",__FUNCTION__,m_rec.ID());
      m_eventHandler.SetRecorder(m_rec);
      MythChannel chan=m_channels[m_channelsMap[channel.iUniqueId]];
      return m_rec.SpawnLiveTV(chan);
    }
    else
      return true;

  }


  void PVRClientMythTV::CloseLiveStream()
{
  m_rec.Stop();
  m_rec=NULL;
  return;
}

int PVRClientMythTV::ReadLiveStream(unsigned char *pBuffer, unsigned int iBufferSize)
{
  if(m_rec.IsNull())
    return -1;
  return m_rec.ReadLiveTV(pBuffer,iBufferSize);
}

int PVRClientMythTV::GetCurrentClientChannel()
{
  if(m_rec.IsNull())
    return -1;
  MythProgramInfo currentProgram=m_rec.GetCurrentProgram();
  CStdString channel=currentProgram.ChannelName();
  for(std::vector<MythChannel>::iterator it=m_channels.begin();it!=m_channels.end();it++)
  {
    if(it->Name()==channel)
      return it->ID();
  }
  return -1;
}

bool PVRClientMythTV::SwitchChannel(const PVR_CHANNEL &channelinfo)
{
  MythChannel chan=m_channels[m_channelsMap[channelinfo.iUniqueId]];
  return m_rec.SetChannel(chan);
}

long long PVRClientMythTV::SeekLiveStream(long long iPosition, int iWhence) { 
  int whence=iWhence==SEEK_SET?WHENCE_SET:iWhence==SEEK_CUR?WHENCE_CUR:WHENCE_END;
  return m_rec.LiveTVSeek(iPosition,whence);
  }

long long PVRClientMythTV::LengthLiveStream()
{
  return m_rec.LiveTVDuration();
}

PVR_ERROR PVRClientMythTV::SignalStatus(PVR_SIGNAL_STATUS &signalStatus)
{
  MythSignal signal=m_eventHandler.GetSignal();
  signalStatus.dAudioBitrate=0;
  signalStatus.dDolbyBitrate=0;
  signalStatus.dVideoBitrate=0;
  signalStatus.iBER=signal.BER();
  signalStatus.iSignal=signal.Signal();
  signalStatus.iSNR=signal.SNR();
  signalStatus.iUNC=signal.UNC();
  CStdString ID;
  CStdString adaptorStatus=signal.AdapterStatus();
  ID.Format("Myth Recorder %i",m_rec.ID());
  strcpy(signalStatus.strAdapterName,ID.Buffer());
  strcpy(signalStatus.strAdapterStatus,adaptorStatus.Buffer());
  return PVR_ERROR_NO_ERROR;
}

bool PVRClientMythTV::OpenRecordedStream(const PVR_RECORDING &recinfo)
{
  //long long uid=*reinterpret_cast<const long long*>(recinfo.strRecordingId);
  CStdString id=recinfo.strRecordingId;
  m_file=m_con.ConnectFile(m_recordings[id]);

  return !m_file.IsNull();
}

void PVRClientMythTV::CloseRecordedStream()
{
  m_file=0;
}

int PVRClientMythTV::ReadRecordedStream(unsigned char *pBuffer, unsigned int iBufferSize)
{
  return m_file.Read(pBuffer,iBufferSize);
}

long long PVRClientMythTV::SeekRecordedStream(long long iPosition, int iWhence)
{
  int whence=iWhence==SEEK_SET?WHENCE_SET:iWhence==SEEK_CUR?WHENCE_CUR:WHENCE_END;
	return m_file.Seek(iPosition,whence);
}


long long PVRClientMythTV::LengthRecordedStream()
{
  return m_file.Duration();
}


  int PVRClientMythTV::GetChannelGroupsAmount()
    {
  return 2;
 }

  PVR_ERROR PVRClientMythTV::GetChannelGroups(PVR_HANDLE handle, bool bRadio)
    {
      PVR_CHANNEL_GROUP tag;
      tag.bIsRadio=bRadio;
      tag.strGroupName=bRadio?"radio":"tv";
      PVR->TransferChannelGroup(handle,&tag);
      return PVR_ERROR_NO_ERROR;
    
  }
  PVR_ERROR PVRClientMythTV::GetChannelGroupMembers(PVR_HANDLE handle, const PVR_CHANNEL_GROUP &group)
    {
    PVR_CHANNEL_GROUP_MEMBER tag;
    for(std::vector<MythChannel>::iterator it=m_channels.begin();it!=m_channels.end();it++)
    {
      if(it->IsRadio()==group.bIsRadio)
      {
        tag.iChannelNumber=it->Number();
        tag.iChannelUniqueId=it->ID();
        tag.strGroupName=group.strGroupName;
        PVR->TransferChannelGroupMember(handle,&tag);
      }
    }
    return PVR_ERROR_NO_ERROR;
  }