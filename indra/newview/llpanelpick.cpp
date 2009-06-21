/** 
 * @file llpanelpick.cpp
 * @brief LLPanelPick class implementation
 *
 * $LicenseInfo:firstyear=2004&license=viewergpl$
 * 
 * Copyright (c) 2004-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

// Display of a "Top Pick" used both for the global top picks in the 
// Find directory, and also for each individual user's picks in their
// profile.

#include "llviewerprecompiledheaders.h"
#include "llpanel.h"
#include "message.h"
#include "llagent.h"
#include "llparcel.h"
#include "llviewerparcelmgr.h"
#include "lltexturectrl.h"
#include "lluiconstants.h"
#include "llavatarpropertiesprocessor.h"
#include "llpanelpick.h"
#include "llpanelmeprofile.h"


#define XML_PANEL_EDIT_PICK "panel_edit_pick.xml"
#define XML_PANEL_PICK_INFO "panel_pick_info.xml"

#define XML_NAME		"pick_name"
#define XML_DESC		"pick_desc"
#define XML_SNAPSHOT	"pick_snapshot"
#define XML_LOCATION	"pick_location"


LLPanelPick::LLPanelPick(BOOL edit_mode/* = FALSE */)
:	LLPanel(), LLAvatarPropertiesObserver(),
	mEditMode(edit_mode),
	mSnapshotCtrl(NULL),
	mPickId(LLUUID::null),
	mCreatorId(LLUUID::null),
	mDataReceived(FALSE)
{
	if (edit_mode)
	{
		LLUICtrlFactory::getInstance()->buildPanel(this, XML_PANEL_EDIT_PICK);
		LLAvatarPropertiesProcessor::instance().addObserver(gAgentID, this);
	}
	else
	{
		LLUICtrlFactory::getInstance()->buildPanel(this, XML_PANEL_PICK_INFO);
	}

}

LLPanelPick::~LLPanelPick()
{
	if (!mCreatorId.isNull()) 	LLAvatarPropertiesProcessor::instance().removeObserver(mCreatorId, this);
}

void LLPanelPick::reset()
{
	mPickId.setNull();
	mCreatorId.setNull();
	mParcelId.setNull();
	
	mDataReceived = FALSE;

	mPosGlobal.clearVec();
}

BOOL LLPanelPick::postBuild()
{
	mSnapshotCtrl = getChild<LLTextureCtrl>(XML_SNAPSHOT);

	if (mEditMode)
	{
		childSetAction("cancel_btn", onClickCancel, this);
		childSetAction("set_to_curr_location_btn", onClickSet, this);
		childSetAction("save_changes_btn", onClickSave, this);
	}
	else
	{
		childSetAction("edit_btn", onClickEdit, this);
		childSetAction("teleport_btn", onClickTeleport, this);
		childSetAction("show_on_map_btn", onClickMap, this);
		childSetAction("back_btn", onClickBack, this);
		//*TODO set on menu
	}

	return TRUE;
}

void LLPanelPick::init(LLUUID creator_id, LLUUID pick_id)
{
	mCreatorId = creator_id;
	mPickId = pick_id;

	// on Pick Info panel (for non-Agent picks) edit_btn should be invisible
	if (!mEditMode)
	{
		if (mCreatorId != gAgentID)
		{
			childSetEnabled("edit_btn", FALSE);
			childSetVisible("edit_btn", FALSE);
		}
		else 
		{
			childSetEnabled("edit_btn", TRUE);
			childSetVisible("edit_btn", TRUE);
		}
	}

	LLAvatarPropertiesProcessor::instance().addObserver(mCreatorId, this);
	LLAvatarPropertiesProcessor::instance().sendDataRequest(mCreatorId, APT_PICK_INFO, &mPickId);
}

void LLPanelPick::init(LLPickData *pick_data)
{
	mPickId = pick_data->pick_id;
	mCreatorId = pick_data->creator_id;

	setName(pick_data->name);
	setDesc(pick_data->desc);
	setLocation(pick_data->location_text);
	mSnapshotCtrl->setImageAssetID(pick_data->snapshot_id);

	mPosGlobal = pick_data->pos_global;
	mSimName = pick_data->sim_name;
	mParcelId = pick_data->parcel_id;
}

// Fill in some reasonable defaults for a new pick.
void LLPanelPick::createNewPick()
{
	mPickId.generate();
	mCreatorId = gAgent.getID();
	mPosGlobal = gAgent.getPositionGlobal();

	// Try to fill in the current parcel
	LLParcel* parcel = LLViewerParcelMgr::getInstance()->getAgentParcel();
	if (parcel)
	{
		setName(parcel->getName());
		setDesc(parcel->getDesc());
		mSnapshotCtrl->setImageAssetID(parcel->getSnapshotID());
	}

	sendUpdate();
}

/*virtual*/ void LLPanelPick::processProperties(void* data, EAvatarProcessorType type)
{
	if (APT_PICK_INFO != type) return;
	if (!data) return;

	LLPickData* pick_data = static_cast<LLPickData *>(data);
	if (!pick_data) return;
	if (mPickId != pick_data->pick_id) return;

	init(pick_data);
	mDataReceived = TRUE;
	LLAvatarPropertiesProcessor::instance().removeObserver(gAgentID, this);
}


void LLPanelPick::setEditMode( BOOL edit_mode )
{
	if (mEditMode == edit_mode) return;
	mEditMode = edit_mode;

	if (edit_mode)
	{
		// preserve data before killing controls
		std::string name = getName();
		std::string desc = getDesc();
		std::string location = getLocation();
		LLUUID snapshot_id = mSnapshotCtrl->getImageAssetID();
		LLRect old_rect = getRect();

		deleteAllChildren();

		LLUICtrlFactory::getInstance()->buildPanel(this, XML_PANEL_EDIT_PICK);

		//*NOTE this code is from LLPanelMeProfile.togglePanel()... doubt this is a right way to do things
		reshape(old_rect.getWidth(), old_rect.getHeight());
		old_rect.setLeftTopAndSize(0, old_rect.getHeight(), old_rect.getWidth(), old_rect.getHeight());
		setRect(old_rect);

		// time to restore data
		setName(name);
		setDesc(desc);
		setLocation(location);
		mSnapshotCtrl->setImageAssetID(snapshot_id);
	}
	else
	{
		// returning to VIEW mode - need to perform cleanup 
		// this is the case when that panel is reused between viewing/editing different picks
		deleteAllChildren();
		reset();
		LLUICtrlFactory::getInstance()->buildPanel(this, XML_PANEL_PICK_INFO);
	}
}

//*HACK need to be redone - control panel toggling from parent (Me Panel/Avatar Profile Panel)
void LLPanelPick::setPanelMeProfile(LLPanelMeProfile* meProfilePanel)
{
	mMeProfilePanel = meProfilePanel;
}

void LLPanelPick::setName(std::string name)
{
	if (mEditMode)
	{
		childSetValue(XML_NAME, name);
	}
	else
	{
		childSetWrappedText(XML_NAME, name);
	}
}

void LLPanelPick::setDesc(std::string desc)
{
	if (mEditMode)
	{
		childSetValue(XML_DESC, desc);
	}
	else
	{
		childSetWrappedText(XML_DESC, desc);
	}
}

void LLPanelPick::setLocation(std::string location)
{
	childSetWrappedText(XML_LOCATION, location);
}

std::string LLPanelPick::getName()
{
	return childGetValue(XML_NAME).asString();
}

std::string LLPanelPick::getDesc()
{
	return childGetValue(XML_DESC).asString();
}

std::string LLPanelPick::getLocation()
{
	return childGetValue(XML_LOCATION).asString();
}

void LLPanelPick::sendUpdate()
{
	LLPickData pick_data;

	// If we don't have a pick id yet, we'll need to generate one,
	// otherwise we'll keep overwriting pick_id 00000 in the database.
	if (mPickId.isNull()) mPickId.generate();

	pick_data.agent_id = gAgent.getID();
	pick_data.session_id = gAgent.getSessionID();
	pick_data.pick_id = mPickId;
	pick_data.creator_id = gAgentID;

	//legacy var  need to be deleted
	pick_data.top_pick = FALSE; 
	pick_data.parcel_id = mParcelId;
	pick_data.name = getName();
	pick_data.desc = getDesc();
	pick_data.snapshot_id = mSnapshotCtrl->getImageAssetID();
	pick_data.pos_global = mPosGlobal;
	pick_data.sort_order = 0;
	pick_data.enabled = TRUE;

	LLAvatarPropertiesProcessor::instance().sendDataUpdate(&pick_data, APT_PICK_INFO);
}


//-----------------------------------------
// "PICK INFO" (VIEW MODE) BUTTON HANDLERS
//-----------------------------------------

//static
void LLPanelPick::onClickEdit(void* data)
{
	LLPanelPick* self = (LLPanelPick*)data;
	if (!self) return;
	if (self->mEditMode) return;
	if (!self->mDataReceived) return;

	self->setEditMode(TRUE);
}

//static
void LLPanelPick::onClickTeleport(void* data)
{
	//LLPanelPick* self = (LLPanelPick*)data;
	//*TODO implement
}

//static
void LLPanelPick::onClickMap(void* data)
{
	//LLPanelPick* self = (LLPanelPick*)data;
	//*TODO implement
}

//*HACK need to move panel toggling to parent panels
//static
void LLPanelPick::onClickBack(void* data)
{
	LLPanelPick* self = (LLPanelPick*)data;
	if (!self) return;
	self->mMeProfilePanel->togglePanel(self);
}



//-----------------------------------------
// "EDIT PICK" (EDIT MODE) BUTTON HANDLERS
//-----------------------------------------

//static
void LLPanelPick::onClickCancel(void* data)
{
	LLPanelPick* self = (LLPanelPick*) data;
	if (!self) return;
	if (!self->mEditMode) return;
	self->mMeProfilePanel->togglePanel(self);
}

// static
void LLPanelPick::onClickSet(void* data)
{
	//TODO check whether pick data was received before

	LLPanelPick* self = (LLPanelPick*) data;
	if (!self) return;
	if (!self->mEditMode) return;

	// Save location for later.
	self->mPosGlobal = gAgent.getPositionGlobal();

	S32 region_x = llround((F32)self->mPosGlobal.mdV[VX]) % REGION_WIDTH_UNITS;
	S32 region_y = llround((F32)self->mPosGlobal.mdV[VY]) % REGION_WIDTH_UNITS;
	S32 region_z = llround((F32)self->mPosGlobal.mdV[VZ]);

	std::string location_text = "(will update after save), ";
	location_text.append(self->mSimName);
	location_text.append(llformat(" (%d, %d, %d)", region_x, region_y, region_z));

	self->setLocation(location_text);
}

// static
void LLPanelPick::onClickSave(void* data)
{
	LLPanelPick* self = (LLPanelPick*)data;
	if (!self->mEditMode) return;
	if (!self->mDataReceived) return;

	//*TODO check if data was received before 
	self->sendUpdate();
	self->mMeProfilePanel->togglePanel(self);
}
