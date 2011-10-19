#include "MythTimer.h"

#include "client.h"

using namespace ADDON;

/*
 *              MythTimer
 */


MythTimer::MythTimer()
  : m_recordid(-1),m_chanid(-1),m_channame(""),m_starttime(0),m_endtime(0),m_title(""),m_description(""),m_type(NotRecording),m_category(""),m_subtitle(""),m_priority(0),m_startoffset(0),m_endoffset(0),m_searchtype(NoSearch),m_inactive(true)
{}


  MythTimer::MythTimer(cmyth_timer_t cmyth_timer,bool release)
    : m_recordid(CMYTH->TimerRecordid(cmyth_timer)),
    m_chanid(CMYTH->TimerChanid(cmyth_timer)),
    m_channame(""),
    m_starttime(CMYTH->TimerStarttime(cmyth_timer)),
    m_endtime(CMYTH->TimerEndtime(cmyth_timer)),
    m_title(""),
    m_description(""),
    m_type(static_cast<TimerType>(CMYTH->TimerType(cmyth_timer))),
    m_category(""),
    m_subtitle(""),
    m_priority(CMYTH->TimerPriority(cmyth_timer)),
    m_startoffset(CMYTH->TimerStartoffset(cmyth_timer)),
    m_endoffset(CMYTH->TimerEndoffset(cmyth_timer)),
    m_searchtype(static_cast<TimerSearchType>(CMYTH->TimerSearchtype(cmyth_timer))),
    m_inactive(CMYTH->TimerInactive(cmyth_timer)!=0)
  {
    char *title = CMYTH->TimerTitle(cmyth_timer);
    char *description = CMYTH->TimerDescription(cmyth_timer);
    char *category = CMYTH->TimerCategory(cmyth_timer);
    char *subtitle = CMYTH->TimerSubtitle(cmyth_timer);
    char *channame = CMYTH->TimerChanname(cmyth_timer);
    m_title = title;
    m_description = description;
    m_category = category;
    m_subtitle = subtitle;
    m_channame = channame;
    CMYTH->RefRelease(title);
    CMYTH->RefRelease(description);
    CMYTH->RefRelease(category);
    CMYTH->RefRelease(subtitle);
    CMYTH->RefRelease(channame);
    if(release)
      CMYTH->RefRelease(cmyth_timer);
  }

  int MythTimer::RecordID() const
  {
    return m_recordid;
  }

  void MythTimer::RecordID(int recordid)
  {
    m_recordid=recordid;
  }

  int MythTimer::ChanID() const
  {
    return m_chanid;
  }

  void MythTimer::ChanID(int chanid)
  {
    m_chanid = chanid;
  }

  CStdString MythTimer::ChanName() const
  {
    return m_channame;
  }

  void MythTimer::ChanName(const CStdString& channame)
  {
    m_channame = channame;
  }

  time_t MythTimer::StartTime() const
  {
    return m_starttime;
  }

  void MythTimer::StartTime(time_t starttime)
  {
    m_starttime=starttime;
  }

  time_t MythTimer::EndTime() const
  {
    return m_endtime;
  }

  void MythTimer::EndTime(time_t endtime)
  {
    m_endtime=endtime;
  }

  CStdString MythTimer::Title() const
  {
    return m_title;
  }

  void MythTimer::Title(const CStdString& title)
  {
    m_title=title;
  }

  CStdString MythTimer::Subtitle() const
  {
    return m_subtitle;
  }

  void MythTimer::Subtitle(const CStdString& subtitle)
  {
    m_subtitle=subtitle;
  }

  CStdString MythTimer::Description() const
   {
    return m_description;
  }

   void MythTimer::Description(const CStdString& description)
   {
     m_description=description;
   }

  MythTimer::TimerType MythTimer::Type() const
  {
    return m_type;
  }

   void MythTimer::Type(MythTimer::TimerType type)
   {
     m_type=type;
   }

  CStdString MythTimer::Category() const
  {
    return m_category;
  }

  void MythTimer::Category(const CStdString& category)
  {
    m_category=category;
  }

  int MythTimer::StartOffset() const
  {
    return m_startoffset;
  }
  
    void MythTimer::StartOffset(int startoffset)
    {
      m_startoffset=startoffset;
    }

  int MythTimer::EndOffset() const
  {
    return m_endoffset;
  }

    void MythTimer::EndOffset(int endoffset)
    {
      m_endoffset=endoffset;
    }

  int MythTimer::Priority() const
  {
    return m_priority;
  }

    void MythTimer::Priority(int priority)
    {
      m_priority=priority;
    }

  bool MythTimer::Inactive() const
  {
    return m_inactive;
  }

  void MythTimer::Inactive(bool inactive)
  {
    m_inactive=inactive;
  }

  MythTimer::TimerSearchType MythTimer::SearchType() const
  {
    return m_searchtype;
  }

    void MythTimer::SearchType(MythTimer::TimerSearchType searchtype)
    {
      m_searchtype=searchtype;
    }
