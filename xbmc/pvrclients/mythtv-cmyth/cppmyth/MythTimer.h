#pragma once

#include "libcmyth.h"
#include "utils/StdString.h"

class MythTimer
{
  friend class MythDatabase;
public:
  MythTimer();
  MythTimer(cmyth_timer_t cmyth_timer,bool release=true);
  int RecordID() const;
  void RecordID(int recordid);
  int ChanID() const;
  void ChanID(int chanid);
  CStdString ChanName() const;
  void ChanName(const CStdString& channame);
  time_t StartTime() const;
  void StartTime(time_t starttime);
  time_t EndTime() const;
  void EndTime(time_t endtime);
  CStdString Title() const;
  void Title(const CStdString& title);
  CStdString Subtitle() const;
  void Subtitle(const CStdString& subtitle);
  CStdString Description() const;
  void Description(const CStdString& description);
  typedef enum TimerTypes
{
    NotRecording = 0,
    SingleRecord = 1,
    TimeslotRecord,
    ChannelRecord,
    AllRecord,
    WeekslotRecord,
    FindOneRecord,
    OverrideRecord,
    DontRecord,
    FindDailyRecord,
    FindWeeklyRecord
} TimerType;
  TimerType Type() const;
  void Type(TimerType type);
  CStdString Category() const;
  void Category(const CStdString& category);
  int StartOffset() const;
  void StartOffset(int startoffset);
  int EndOffset() const;
  void EndOffset(int endoffset);
  int Priority() const;
  void Priority(int priority);
  bool Inactive() const;
  void Inactive(bool inactive);
typedef enum TimerSearchTypes
{
    NoSearch = 0,
    PowerSearch,
    TitleSearch,
    KeywordSearch,
    PeopleSearch,
    ManualSearch
} TimerSearchType;
  TimerSearchType SearchType() const;
  void SearchType(TimerSearchType searchtype);
private:
  int m_recordid;
  int m_chanid; 
  CStdString m_channame;
  time_t m_starttime;  
  time_t m_endtime;    
	CStdString m_title;
	CStdString m_description;  
  TimerType m_type;      
	CStdString m_category;
  CStdString m_subtitle;
  int m_priority;
  int m_startoffset;
  int m_endoffset;
  TimerSearchType m_searchtype;
  bool m_inactive;
};