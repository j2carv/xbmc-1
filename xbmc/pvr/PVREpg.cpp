/*
 *      Copyright (C) 2005-2010 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "GUISettings.h"
#include "log.h"
#include "TimeUtils.h"

#include "PVREpg.h"
#include "PVRChannel.h"
#include "PVRManager.h"

#include "epg/EpgContainer.h"
#include "epg/EpgDatabase.h"

CPVREpg::CPVREpg(CPVRChannel *channel) :
  CEpg(channel->ChannelID(), channel->EPGScraper())
{
  m_Channel  = channel;
  m_bIsRadio = channel->IsRadio();
}

bool CPVREpg::HasValidEntries(void) const
{
  bool bReturn = CEpg::HasValidEntries();

  if (bReturn)
    bReturn = (m_Channel->ChannelID() > 0); /* valid channel ID */

  return bReturn;
}

void CPVREpg::Cleanup(const CDateTime Time)
{
  SetUpdateRunning(true);
  for (unsigned int i = 0; i < size(); i++)
  {
    CPVREpgInfoTag *tag = (CPVREpgInfoTag *) at(i);
    if ( tag && /* valid tag */
        !tag->HasTimer() && /* no time set */
        (tag->End() + CDateTimeSpan(0, g_PVREpgContainer.m_iLingerTime / 60 + 1, g_PVREpgContainer.m_iLingerTime % 60, 0) < Time)) /* adding one hour for safety */
    {
      DeleteInfoTag(tag);
    }
  }
  SetUpdateRunning(false);
}

bool CPVREpg::UpdateEntry(const PVR_PROGINFO *data, bool bUpdateDatabase /* = false */)
{
  if (!data)
    return false;

  CPVREpgInfoTag *InfoTag = (CPVREpgInfoTag *) this->InfoTag(data->uid, data->starttime);

  if (InfoTag == NULL)
  {
    InfoTag = new CPVREpgInfoTag(*data);
    push_back(InfoTag);
  }

  return CEpg::UpdateEntry(*InfoTag, bUpdateDatabase);
}

bool CPVREpg::UpdateFromClient(time_t start, time_t end)
{
  bool bGrabSuccess = false;

  if (g_PVRManager.GetClientProps(m_Channel->ClientID())->SupportEPG &&
      g_PVRManager.Clients()->find(m_Channel->ClientID())->second->ReadyToUse())
  {
    bGrabSuccess = g_PVRManager.Clients()->find(m_Channel->ClientID())->second->GetEPGForChannel(*m_Channel, this, start, end) == PVR_ERROR_NO_ERROR;
  }
  else
  {
    CLog::Log(LOGINFO, "%s - client '%s' on client '%i' does not support EPGs",
        __FUNCTION__, m_Channel->ChannelName().c_str(), m_Channel->ClientID());
  }

  return bGrabSuccess;
}

bool CPVREpg::LoadFromDb()
{
  /* check if this channel is marked for grabbing */
  if (!m_Channel || !m_Channel->EPGEnabled())
    return false;

  return CEpg::LoadFromDb();
}

bool CPVREpg::Update(time_t start, time_t end, bool bStoreInDb /* = true */) // XXX add locking
{
  /* check if this channel is marked for grabbing */
  if (!m_Channel || !m_Channel->EPGEnabled())
    return false;

  bool bGrabSuccess = true;
  CEpgDatabase *database = g_PVREpgContainer.GetDatabase();

  /* mark the EPG as being updated */
  SetUpdateRunning(true);

  bGrabSuccess = (ScraperName() == "client") ?
      UpdateFromClient(start, end) || bGrabSuccess:
      UpdateFromScraper(start, end) || bGrabSuccess;

  /* store the loaded EPG entries in the database */
  if (bGrabSuccess)
  {
    FixOverlappingEvents(bStoreInDb);

    if (bStoreInDb)
    {
      for (unsigned int iTagPtr = 0; iTagPtr < size(); iTagPtr++)
        database->UpdateEpgEntry(*at(iTagPtr), false, (iTagPtr == size() - 1));
    }
  }

  SetUpdateRunning(false);

  return bGrabSuccess;
}