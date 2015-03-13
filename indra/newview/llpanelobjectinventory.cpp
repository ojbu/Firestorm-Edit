/**
 * @file llsidepanelinventory.cpp
 * @brief LLPanelObjectInventory class implementation
 *
 * $LicenseInfo:firstyear=2002&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

//*****************************************************************************
//
// Implementation of the panel inventory - used to view and control a
// task's inventory.
//
//*****************************************************************************

#include "llviewerprecompiledheaders.h"

#include "llpanelobjectinventory.h"

#include "llmenugl.h"
#include "llnotificationsutil.h"
#include "roles_constants.h"

#include "llagent.h"
#include "llavataractions.h"
#include "llcallbacklist.h"
#include "llbuycurrencyhtml.h"
#include "llfloaterreg.h"
#include "llfolderview.h"
#include "llinventorybridge.h"
#include "llinventorydefines.h"
#include "llinventoryicon.h"
#include "llinventoryfilter.h"
#include "llinventoryfunctions.h"
#include "llpreviewanim.h"
#include "llpreviewgesture.h"
#include "llpreviewnotecard.h"
#include "llpreviewscript.h"
#include "llpreviewsound.h"
#include "llpreviewtexture.h"
#include "llscrollcontainer.h"
#include "llselectmgr.h"
#include "llstatusbar.h"
#include "lltooldraganddrop.h"
#include "lltrans.h"
#include "llviewerassettype.h"
#include "llviewerinventory.h"
#include "llviewerregion.h"
#include "llviewerobjectlist.h"
#include "llviewermessage.h"
// [RLVa:KB] - Checked: 2011-05-22 (RLVa-1.3.1a)
#include "rlvhandler.h"
#include "rlvlocks.h"
// [/RLVa:KB]

const LLColor4U DEFAULT_WHITE(255, 255, 255);
#include "tea.h" // <FS:AW opensim currency support>

///----------------------------------------------------------------------------
/// Class LLTaskInvFVBridge
///----------------------------------------------------------------------------

class LLTaskInvFVBridge : public LLFolderViewModelItemInventory
{
protected:
	LLUUID mUUID;
	std::string mName;
	mutable std::string mDisplayName;
	mutable std::string mSearchableName;
	LLPanelObjectInventory* mPanel;
	U32 mFlags;
	LLAssetType::EType mAssetType;	
	LLInventoryType::EType mInventoryType;

	LLInventoryObject* findInvObject() const;
	LLInventoryItem* findItem() const;

public:
	LLTaskInvFVBridge(LLPanelObjectInventory* panel,
					  const LLUUID& uuid,
					  const std::string& name,
					  U32 flags=0);
	virtual ~LLTaskInvFVBridge() {}

	virtual LLFontGL::StyleFlags getLabelStyle() const { return LLFontGL::NORMAL; }
	virtual std::string getLabelSuffix() const { return LLStringUtil::null; }

	static LLTaskInvFVBridge* createObjectBridge(LLPanelObjectInventory* panel,
												 LLInventoryObject* object);
	void showProperties();
	void buyItem();
	S32 getPrice();
	static bool commitBuyItem(const LLSD& notification, const LLSD& response);

	// LLFolderViewModelItemInventory functionality
	virtual const std::string& getName() const;
	virtual const std::string& getDisplayName() const;
	virtual const std::string& getSearchableName() const;

	virtual PermissionMask getPermissionMask() const { return PERM_NONE; }
	/*virtual*/ LLFolderType::EType getPreferredType() const { return LLFolderType::FT_NONE; }
	virtual const LLUUID& getUUID() const { return mUUID; }
	virtual time_t getCreationDate() const;
	virtual void setCreationDate(time_t creation_date_utc);

	virtual LLUIImagePtr getIcon() const;
	virtual void openItem();
	virtual BOOL canOpenItem() const { return FALSE; }
	virtual void closeItem() {}
	virtual void selectItem() {}
	virtual BOOL isItemRenameable() const;
	virtual BOOL renameItem(const std::string& new_name);
	virtual BOOL isItemMovable() const;
	virtual BOOL isItemRemovable() const;
	virtual BOOL removeItem();
	virtual void removeBatch(std::vector<LLFolderViewModelItem*>& batch);
	virtual void move(LLFolderViewModelItem* parent_listener);	
	virtual BOOL isItemCopyable() const;
	virtual BOOL copyToClipboard() const;
	virtual BOOL cutToClipboard() const;
	virtual BOOL isClipboardPasteable() const;
	virtual void pasteFromClipboard();
	virtual void pasteLinkFromClipboard();
	virtual void buildContextMenu(LLMenuGL& menu, U32 flags);
	virtual void performAction(LLInventoryModel* model, std::string action);
	virtual BOOL isUpToDate() const { return TRUE; }
	virtual bool hasChildren() const { return FALSE; }
	virtual LLInventoryType::EType getInventoryType() const { return LLInventoryType::IT_NONE; }
	virtual LLWearableType::EType getWearableType() const { return LLWearableType::WT_NONE; }
	virtual EInventorySortGroup getSortGroup() const { return SG_ITEM; }
	virtual LLInventoryObject* getInventoryObject() const { return findInvObject(); }


	// LLDragAndDropBridge functionality
	virtual LLToolDragAndDrop::ESource getDragSource() const { return LLToolDragAndDrop::SOURCE_WORLD; }
	virtual BOOL startDrag(EDragAndDropType* type, LLUUID* id) const;
	virtual BOOL dragOrDrop(MASK mask, BOOL drop,
							EDragAndDropType cargo_type,
							void* cargo_data,
							std::string& tooltip_msg);
};

LLTaskInvFVBridge::LLTaskInvFVBridge(
	LLPanelObjectInventory* panel,
	const LLUUID& uuid,
	const std::string& name,
	U32 flags)
:	LLFolderViewModelItemInventory(panel->getRootViewModel()),
	mUUID(uuid),
	mName(name),
	mPanel(panel),
	mFlags(flags),
	mAssetType(LLAssetType::AT_NONE),
	mInventoryType(LLInventoryType::IT_NONE)
{
	const LLInventoryItem *item = findItem();
	if (item)
	{
		mAssetType = item->getType();
		mInventoryType = item->getInventoryType();
	}
}

LLInventoryObject* LLTaskInvFVBridge::findInvObject() const
{
	LLViewerObject* object = gObjectList.findObject(mPanel->getTaskUUID());
	if (object)
	{
		return object->getInventoryObject(mUUID);
	}
	return NULL;
}


LLInventoryItem* LLTaskInvFVBridge::findItem() const
{
	return dynamic_cast<LLInventoryItem*>(findInvObject());
}

void LLTaskInvFVBridge::showProperties()
{
	//show_task_item_profile(mUUID, mPanel->getTaskUUID());
	//-TT Experiment.
	LLFloaterReg::showInstance("properties", LLSD().with("item_id", mUUID).with("object_id", mPanel->getTaskUUID()));
}

struct LLBuyInvItemData
{
	LLUUID mTaskID;
	LLUUID mItemID;
	LLAssetType::EType mType;

	LLBuyInvItemData(const LLUUID& task,
					 const LLUUID& item,
					 LLAssetType::EType type) :
		mTaskID(task), mItemID(item), mType(type)
	{}
};

void LLTaskInvFVBridge::buyItem()
{
	LL_INFOS() << "LLTaskInvFVBridge::buyItem()" << LL_ENDL;
	LLInventoryItem* item = findItem();
	if(!item || !item->getSaleInfo().isForSale()) return;
	LLBuyInvItemData* inv = new LLBuyInvItemData(mPanel->getTaskUUID(),
												 mUUID,
												 item->getType());

	const LLSaleInfo& sale_info = item->getSaleInfo();
	const LLPermissions& perm = item->getPermissions();
	const std::string owner_name; // no owner name currently... FIXME?

	LLViewerObject* obj;
	if( ( obj = gObjectList.findObject( mPanel->getTaskUUID() ) ) && obj->isAttachment() )
	{
		LLNotificationsUtil::add("Cannot_Purchase_an_Attachment");
		LL_INFOS() << "Attempt to purchase an attachment" << LL_ENDL;
		delete inv;
	}
	else
	{
        LLSD args;
        args["PRICE"] = llformat("%d",sale_info.getSalePrice());
        args["OWNER"] = owner_name;
        if (sale_info.getSaleType() != LLSaleInfo::FS_CONTENTS)
        {
        	U32 next_owner_mask = perm.getMaskNextOwner();
        	args["MODIFYPERM"] = LLTrans::getString((next_owner_mask & PERM_MODIFY) ? "PermYes" : "PermNo");
        	args["COPYPERM"] = LLTrans::getString((next_owner_mask & PERM_COPY) ? "PermYes" : "PermNo");
        	args["RESELLPERM"] = LLTrans::getString((next_owner_mask & PERM_TRANSFER) ? "PermYes" : "PermNo");
        }

		std::string alertdesc;
       	switch(sale_info.getSaleType())
       	{
       	  case LLSaleInfo::FS_ORIGINAL:
       		alertdesc = owner_name.empty() ? "BuyOriginalNoOwner" : "BuyOriginal";
       		break;
       	  case LLSaleInfo::FS_CONTENTS:
       		alertdesc = owner_name.empty() ? "BuyContentsNoOwner" : "BuyContents";
       		break;
		  case LLSaleInfo::FS_COPY:
       	  default:
       		alertdesc = owner_name.empty() ? "BuyCopyNoOwner" : "BuyCopy";
       		break;
       	}

		LLSD payload;
		payload["task_id"] = inv->mTaskID;
		payload["item_id"] = inv->mItemID;
		payload["type"] = inv->mType;
		LLNotificationsUtil::add(alertdesc, args, payload, LLTaskInvFVBridge::commitBuyItem);
	}
}

S32 LLTaskInvFVBridge::getPrice()
{
	LLInventoryItem* item = findItem();
	if(item)
	{
		return item->getSaleInfo().getSalePrice();
	}
	else
	{
		return -1;
	}
}

// static
bool LLTaskInvFVBridge::commitBuyItem(const LLSD& notification, const LLSD& response)
{
	S32 option = LLNotificationsUtil::getSelectedOption(notification, response);
	if(0 == option)
	{
		LLViewerObject* object = gObjectList.findObject(notification["payload"]["task_id"].asUUID());
		if(!object || !object->getRegion()) return false;


		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_BuyObjectInventory);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
		msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		msg->nextBlockFast(_PREHASH_Data);
		msg->addUUIDFast(_PREHASH_ObjectID, notification["payload"]["task_id"].asUUID());
		msg->addUUIDFast(_PREHASH_ItemID, notification["payload"]["item_id"].asUUID());
		msg->addUUIDFast(_PREHASH_FolderID,
			gInventory.findCategoryUUIDForType((LLFolderType::EType)notification["payload"]["type"].asInteger()));
		msg->sendReliable(object->getRegion()->getHost());
	}
	return false;
}

const std::string& LLTaskInvFVBridge::getName() const
{
	return mName;
}

const std::string& LLTaskInvFVBridge::getDisplayName() const
{
	LLInventoryItem* item = findItem();

	if(item)
	{
		mDisplayName.assign(item->getName());

		// Localize "New Script", "New Script 1", "New Script 2", etc.
		if (item->getType() == LLAssetType::AT_LSL_TEXT &&
			LLStringUtil::startsWith(item->getName(), "New Script"))
		{
			LLStringUtil::replaceString(mDisplayName, "New Script", LLTrans::getString("PanelContentsNewScript"));
		}

		const LLPermissions& perm(item->getPermissions());
		BOOL copy = gAgent.allowOperation(PERM_COPY, perm, GP_OBJECT_MANIPULATE);
		BOOL mod  = gAgent.allowOperation(PERM_MODIFY, perm, GP_OBJECT_MANIPULATE);
		BOOL xfer = gAgent.allowOperation(PERM_TRANSFER, perm, GP_OBJECT_MANIPULATE);

		if(!copy)
		{
			mDisplayName.append(LLTrans::getString("no_copy"));
		}
		if(!mod)
		{
			mDisplayName.append(LLTrans::getString("no_modify"));
		}
		if(!xfer)
		{
			mDisplayName.append(LLTrans::getString("no_transfer"));
		}
	}

	mSearchableName.assign(mDisplayName + getLabelSuffix());

	return mDisplayName;
}

const std::string& LLTaskInvFVBridge::getSearchableName() const
{
	return mSearchableName;
}


// BUG: No creation dates for task inventory
time_t LLTaskInvFVBridge::getCreationDate() const
{
	return 0;
}

void LLTaskInvFVBridge::setCreationDate(time_t creation_date_utc)
{}


LLUIImagePtr LLTaskInvFVBridge::getIcon() const
{
	const BOOL item_is_multi = (mFlags & LLInventoryItemFlags::II_FLAGS_OBJECT_HAS_MULTIPLE_ITEMS);

	return LLInventoryIcon::getIcon(mAssetType, mInventoryType, 0, item_is_multi );
}

void LLTaskInvFVBridge::openItem()
{
	// no-op.
	LL_DEBUGS() << "LLTaskInvFVBridge::openItem()" << LL_ENDL;
}

BOOL LLTaskInvFVBridge::isItemRenameable() const
{
// [RLVa:KB] - Checked: 2010-09-28 (RLVa-1.2.1f) | Modified: RLVa-1.0.5a
	LLViewerObject* object = gObjectList.findObject(mPanel->getTaskUUID());
	if ( (rlv_handler_t::isEnabled()) && (object) && (gRlvAttachmentLocks.isLockedAttachment(object->getRootEdit())) )
	{
		return FALSE;
	}
// [/RLVa:KB]

	if(gAgent.isGodlike()) return TRUE;
//	LLViewerObject* object = gObjectList.findObject(mPanel->getTaskUUID());
	if(object)
	{
		LLInventoryItem* item = (LLInventoryItem*)(object->getInventoryObject(mUUID));
		if(item && gAgent.allowOperation(PERM_MODIFY, item->getPermissions(),
										 GP_OBJECT_MANIPULATE, GOD_LIKE))
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL LLTaskInvFVBridge::renameItem(const std::string& new_name)
{
// [RLVa:KB] - Checked: 2010-09-28 (RLVa-1.2.1f) | Modified: RLVa-1.0.5a
	LLViewerObject* object = gObjectList.findObject(mPanel->getTaskUUID());
	if ( (rlv_handler_t::isEnabled()) && (object) && (gRlvAttachmentLocks.isLockedAttachment(object->getRootEdit())) )
	{
		return FALSE;
	}
// [/RLVa:KB]

//	LLViewerObject* object = gObjectList.findObject(mPanel->getTaskUUID());
	if(object)
	{
		LLViewerInventoryItem* item = NULL;
		item = (LLViewerInventoryItem*)object->getInventoryObject(mUUID);
		if(item && (gAgent.allowOperation(PERM_MODIFY, item->getPermissions(),
										GP_OBJECT_MANIPULATE, GOD_LIKE)))
		{
			LLPointer<LLViewerInventoryItem> new_item = new LLViewerInventoryItem(item);
			new_item->rename(new_name);
			object->updateInventory(
				new_item,
				TASK_INVENTORY_ITEM_KEY,
				false);
		}
	}
	return TRUE;
}

BOOL LLTaskInvFVBridge::isItemMovable() const
{
	//LLViewerObject* object = gObjectList.findObject(mPanel->getTaskUUID());
	//if(object && (object->permModify() || gAgent.isGodlike()))
	//{
	//	return TRUE;
	//}
	//return FALSE;
// [RLVa:KB] - Checked: 2010-04-01 (RLVa-1.2.0c) | Modified: RLVa-1.0.5a
	if (rlv_handler_t::isEnabled())
	{
		const LLViewerObject* pObj = gObjectList.findObject(mPanel->getTaskUUID());
		if (pObj)
		{
			if (gRlvAttachmentLocks.isLockedAttachment(pObj->getRootEdit()))
			{
				return FALSE;
			}
			else if ( (gRlvHandler.hasBehaviour(RLV_BHVR_UNSIT)) || (gRlvHandler.hasBehaviour(RLV_BHVR_SITTP)) )
			{
				if ( (isAgentAvatarValid()) && (gAgentAvatarp->isSitting()) && (gAgentAvatarp->getRoot() == pObj->getRootEdit()) )
					return FALSE;
			}
		}
	}
// [/RLVa:KB]
	return TRUE;
}

BOOL LLTaskInvFVBridge::isItemRemovable() const
{
	const LLViewerObject* object = gObjectList.findObject(mPanel->getTaskUUID());
// [RLVa:KB] - Checked: 2010-04-01 (RLVa-1.2.0c) | Modified: RLVa-1.0.5a
	if ( (object) && (rlv_handler_t::isEnabled()) )
	{
		if (gRlvAttachmentLocks.isLockedAttachment(object->getRootEdit()))
		{
			return FALSE;
		}
		else if ( (gRlvHandler.hasBehaviour(RLV_BHVR_UNSIT)) || (gRlvHandler.hasBehaviour(RLV_BHVR_SITTP)) )
		{
			if ( (isAgentAvatarValid()) && (gAgentAvatarp->isSitting()) && (gAgentAvatarp->getRoot() == object->getRootEdit()) )
				return FALSE;
		}
	}
// [/RLVa:KB]

	if(object
	   && (object->permModify() || object->permYouOwner()))
	{
		return TRUE;
	}
	return FALSE;
}

bool remove_task_inventory_callback(const LLSD& notification, const LLSD& response, LLPanelObjectInventory* panel)
{
	S32 option = LLNotificationsUtil::getSelectedOption(notification, response);
	LLViewerObject* object = gObjectList.findObject(notification["payload"]["task_id"].asUUID());
	if(option == 0 && object)
	{
		// yes
		LLSD::array_const_iterator list_end = notification["payload"]["inventory_ids"].endArray();
		for (LLSD::array_const_iterator list_it = notification["payload"]["inventory_ids"].beginArray();
			list_it != list_end; 
			++list_it)
		{
			object->removeInventory(list_it->asUUID());
		}

		// refresh the UI.
		panel->refresh();
	}
	return false;
}

// helper for remove
// ! REFACTOR ! two_uuids_list_t is also defined in llinventorybridge.h, but differently.
typedef std::pair<LLUUID, std::list<LLUUID> > panel_two_uuids_list_t;
typedef std::pair<LLPanelObjectInventory*, panel_two_uuids_list_t> remove_data_t;
BOOL LLTaskInvFVBridge::removeItem()
{
	if(isItemRemovable() && mPanel)
	{
		LLViewerObject* object = gObjectList.findObject(mPanel->getTaskUUID());
		if(object)
		{
			if(object->permModify())
			{
				// just do it.
				object->removeInventory(mUUID);
				return TRUE;
			}
			else
			{
				LLSD payload;
				payload["task_id"] = mPanel->getTaskUUID();
				payload["inventory_ids"].append(mUUID);
				LLNotificationsUtil::add("RemoveItemWarn", LLSD(), payload, boost::bind(&remove_task_inventory_callback, _1, _2, mPanel));
				return FALSE;
			}
		}
	}
	return FALSE;
}

void   LLTaskInvFVBridge::removeBatch(std::vector<LLFolderViewModelItem*>& batch)
{
	if (!mPanel)
	{
		return;
	}

	LLViewerObject* object = gObjectList.findObject(mPanel->getTaskUUID());
	if (!object)
	{
		return;
	}

	if (!object->permModify())
	{
		LLSD payload;
		payload["task_id"] = mPanel->getTaskUUID();
		for (S32 i = 0; i < (S32)batch.size(); i++)
		{
			LLTaskInvFVBridge* itemp = (LLTaskInvFVBridge*)batch[i];
			payload["inventory_ids"].append(itemp->getUUID());
		}
		LLNotificationsUtil::add("RemoveItemWarn", LLSD(), payload, boost::bind(&remove_task_inventory_callback, _1, _2, mPanel));
		
	}
	else
	{
		for (S32 i = 0; i < (S32)batch.size(); i++)
		{
			LLTaskInvFVBridge* itemp = (LLTaskInvFVBridge*)batch[i];

			if(itemp->isItemRemovable())
			{
				// just do it.
				object->removeInventory(itemp->getUUID());
			}
		}
	}
}

void LLTaskInvFVBridge::move(LLFolderViewModelItem* parent_listener)
{
}

BOOL LLTaskInvFVBridge::isItemCopyable() const
{
	LLInventoryItem* item = findItem();
	if(!item) return FALSE;
	return gAgent.allowOperation(PERM_COPY, item->getPermissions(),
								GP_OBJECT_MANIPULATE);
}

BOOL LLTaskInvFVBridge::copyToClipboard() const
{
	return FALSE;
}

BOOL LLTaskInvFVBridge::cutToClipboard() const
{
	return FALSE;
}

BOOL LLTaskInvFVBridge::isClipboardPasteable() const
{
	return FALSE;
}

void LLTaskInvFVBridge::pasteFromClipboard()
{
}

void LLTaskInvFVBridge::pasteLinkFromClipboard()
{
}

BOOL LLTaskInvFVBridge::startDrag(EDragAndDropType* type, LLUUID* id) const
{
	//LL_INFOS() << "LLTaskInvFVBridge::startDrag()" << LL_ENDL;
	if(mPanel)
	{
		LLViewerObject* object = gObjectList.findObject(mPanel->getTaskUUID());
		if(object)
		{
			LLInventoryItem* inv = NULL;
			if((inv = (LLInventoryItem*)object->getInventoryObject(mUUID)))
			{
				const LLPermissions& perm = inv->getPermissions();
				bool can_copy = gAgent.allowOperation(PERM_COPY, perm,
														GP_OBJECT_MANIPULATE);
// [RLVa:KB] - Checked: 2009-10-10 (RLVa-1.2.1f) | Modified: RLVa-1.0.5a
				// Kind of redundant due to the note below, but in case that ever gets fixed
				if ( (rlv_handler_t::isEnabled()) && (gRlvAttachmentLocks.isLockedAttachment(object->getRootEdit())) )
				{
					return FALSE;
				}
// [/RLVa:KB]
				if (object->isAttachment() && !can_copy)
				{
                    //RN: no copy contents of attachments cannot be dragged out
                    // due to a race condition and possible exploit where
                    // attached objects do not update their inventory items
                    // when their contents are manipulated
                    return FALSE;
				}
				if((can_copy && perm.allowTransferTo(gAgent.getID()))
				   || object->permYouOwner())
//				   || gAgent.isGodlike())

				{
					*type = LLViewerAssetType::lookupDragAndDropType(inv->getType());

					*id = inv->getUUID();
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

BOOL LLTaskInvFVBridge::dragOrDrop(MASK mask, BOOL drop,
								   EDragAndDropType cargo_type,
								   void* cargo_data,
								   std::string& tooltip_msg)
{
	//LL_INFOS() << "LLTaskInvFVBridge::dragOrDrop()" << LL_ENDL;
	return FALSE;
}

// virtual
void LLTaskInvFVBridge::performAction(LLInventoryModel* model, std::string action)
{
	if (action == "task_buy")
	{
		// Check the price of the item.
		S32 price = getPrice();
		if (-1 == price)
		{
			LL_WARNS() << "label_buy_task_bridged_item: Invalid price" << LL_ENDL;
		}
		else
		{
			if (price > 0 && price > gStatusBar->getBalance())
			{
				LLStringUtil::format_map_t args;
				args["AMOUNT"] = llformat("%d", price);
				LLBuyCurrencyHTML::openCurrencyFloater( LLTrans::getString("this_costs", args), price );
			}
			else
			{
				buyItem();
			}
		}
	}
	else if (action == "task_open")
	{
		openItem();
	}
	else if (action == "task_properties")
	{
		showProperties();
	}
}

void LLTaskInvFVBridge::buildContextMenu(LLMenuGL& menu, U32 flags)
{
	LLInventoryItem* item = findItem();
	std::vector<std::string> items;
	std::vector<std::string> disabled_items;
	
	if (!item)
	{
		hide_context_entries(menu, items, disabled_items);
		return;
	}

	if(gAgent.allowOperation(PERM_OWNER, item->getPermissions(),
							 GP_OBJECT_MANIPULATE)
	   && item->getSaleInfo().isForSale())
	{
		items.push_back(std::string("Task Buy"));

		std::string label= LLTrans::getString("Buy");
		// Check the price of the item.
		S32 price = getPrice();
		if (-1 == price)
		{
			LL_WARNS() << "label_buy_task_bridged_item: Invalid price" << LL_ENDL;
		}
		else
		{
			std::ostringstream info;
// <FS:AW opensim currency support>
//			info << LLTrans::getString("BuyforL$") << price;
			info << Tea::wrapCurrency(LLTrans::getString("BuyforL$")) << price;
// </FS:AW opensim currency support>
			label.assign(info.str());
		}

		const LLView::child_list_t *list = menu.getChildList();
		LLView::child_list_t::const_iterator itor;
		for (itor = list->begin(); itor != list->end(); ++itor)
		{
			std::string name = (*itor)->getName();
			LLMenuItemCallGL* menu_itemp = dynamic_cast<LLMenuItemCallGL*>(*itor);
			if (name == "Task Buy" && menu_itemp)
			{
				menu_itemp->setLabel(label);
			}
		}
	}
	else if (canOpenItem())
	{
		items.push_back(std::string("Task Open"));
		if (!isItemCopyable())
		{
			disabled_items.push_back(std::string("Task Open"));
		}
// [RLVa:KB] - Checked: 2010-03-01 (RLVa-1.2.0b) | Modified: RLVa-1.1.0a
		else if (rlv_handler_t::isEnabled())
		{
			LLViewerObject* pAttachObj = gObjectList.findObject(mPanel->getTaskUUID());
			bool fLocked = (pAttachObj) ? gRlvAttachmentLocks.isLockedAttachment(pAttachObj->getRootEdit()) : false;
			if ( ((LLAssetType::AT_NOTECARD == item->getType()) && ((gRlvHandler.hasBehaviour(RLV_BHVR_VIEWNOTE)) || (fLocked))) || 
				 ((LLAssetType::AT_LSL_TEXT == item->getType()) && ((gRlvHandler.hasBehaviour(RLV_BHVR_VIEWSCRIPT)) || (fLocked))) ||
				 ((LLAssetType::AT_TEXTURE == item->getType()) && (gRlvHandler.hasBehaviour(RLV_BHVR_VIEWTEXTURE))) )
			{
				disabled_items.push_back(std::string("Task Open"));
			}
		}
// [/RLVa:KB]
	}
	items.push_back(std::string("Task Properties"));
//	if(isItemRenameable())
//	{
//		items.push_back(std::string("Task Rename"));
//		if ((flags & FIRST_SELECTED_ITEM) == 0)
//		{
//			disabled_items.push_back(std::string("Task Rename"));
//		}
//	}
//	if(isItemRemovable())
//	{
//		items.push_back(std::string("Task Remove"));
//	}
// [RLVa:KB] - Checked: 2010-09-28 (RLVa-1.2.1f) | Added: RLVa-1.2.1f
	items.push_back(std::string("Task Rename"));
	items.push_back(std::string("Task Remove"));
	if (!isItemRenameable())
	{
		disabled_items.push_back(std::string("Task Rename"));
	}
	if (!isItemRemovable())
	{
		disabled_items.push_back(std::string("Task Remove"));
	}
// [/RLVa:KB]

	hide_context_entries(menu, items, disabled_items);
}


///----------------------------------------------------------------------------
/// Class LLTaskFolderBridge
///----------------------------------------------------------------------------

class LLTaskCategoryBridge : public LLTaskInvFVBridge
{
public:
	LLTaskCategoryBridge(
		LLPanelObjectInventory* panel,
		const LLUUID& uuid,
		const std::string& name);

	virtual LLUIImagePtr getIcon() const;
	virtual const std::string& getDisplayName() const;
	virtual BOOL isItemRenameable() const;
	// virtual BOOL isItemCopyable() const { return FALSE; }
	virtual BOOL renameItem(const std::string& new_name);
	virtual BOOL isItemRemovable() const;
	virtual void buildContextMenu(LLMenuGL& menu, U32 flags);
	virtual bool hasChildren() const;
	virtual BOOL startDrag(EDragAndDropType* type, LLUUID* id) const;
	virtual BOOL dragOrDrop(MASK mask, BOOL drop,
							EDragAndDropType cargo_type,
							void* cargo_data,
							std::string& tooltip_msg);
	virtual BOOL canOpenItem() const { return TRUE; }
	virtual void openItem();
	virtual EInventorySortGroup getSortGroup() const { return SG_NORMAL_FOLDER; }
};

LLTaskCategoryBridge::LLTaskCategoryBridge(
	LLPanelObjectInventory* panel,
	const LLUUID& uuid,
	const std::string& name) :
	LLTaskInvFVBridge(panel, uuid, name)
{
}

LLUIImagePtr LLTaskCategoryBridge::getIcon() const
{
	return LLUI::getUIImage("Inv_FolderClosed");
}

// virtual
const std::string& LLTaskCategoryBridge::getDisplayName() const
{
	LLInventoryObject* cat = findInvObject();

	if (cat)
	{
		// <FS:Ansariel> Make object root folder name localizable again
		//mDisplayName.assign(cat->getName());
		if (cat->getParentUUID().isNull() && cat->getName() == "Contents")
		{
			mDisplayName.assign(LLTrans::getString("Contents"));
		}
		else
		{
			mDisplayName.assign(cat->getName());
		}
		// </FS:Ansariel>
	}

	return mDisplayName;
}

BOOL LLTaskCategoryBridge::isItemRenameable() const
{
	return FALSE;
}

BOOL LLTaskCategoryBridge::renameItem(const std::string& new_name)
{
	return FALSE;
}

BOOL LLTaskCategoryBridge::isItemRemovable() const
{
	return FALSE;
}

void LLTaskCategoryBridge::buildContextMenu(LLMenuGL& menu, U32 flags)
{
	std::vector<std::string> items;
	std::vector<std::string> disabled_items;
	hide_context_entries(menu, items, disabled_items);
}

bool LLTaskCategoryBridge::hasChildren() const
{
	// return TRUE if we have or do know know if we have children.
	// *FIX: For now, return FALSE - we will know for sure soon enough.
	return FALSE;
}

void LLTaskCategoryBridge::openItem()
{
}

BOOL LLTaskCategoryBridge::startDrag(EDragAndDropType* type, LLUUID* id) const
{
	//LL_INFOS() << "LLTaskInvFVBridge::startDrag()" << LL_ENDL;
	if(mPanel && mUUID.notNull())
	{
		LLViewerObject* object = gObjectList.findObject(mPanel->getTaskUUID());
		if(object)
		{
			const LLInventoryObject* cat = object->getInventoryObject(mUUID);
			if ( (cat) && (move_inv_category_world_to_agent(mUUID, LLUUID::null, FALSE)) )
			{
				*type = LLViewerAssetType::lookupDragAndDropType(cat->getType());
				*id = mUUID;
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL LLTaskCategoryBridge::dragOrDrop(MASK mask, BOOL drop,
									  EDragAndDropType cargo_type,
									  void* cargo_data,
									  std::string& tooltip_msg)
{
	//LL_INFOS() << "LLTaskCategoryBridge::dragOrDrop()" << LL_ENDL;
	BOOL accept = FALSE;
	LLViewerObject* object = gObjectList.findObject(mPanel->getTaskUUID());
	if(object)
	{
		switch(cargo_type)
		{
		case DAD_CATEGORY:
			accept = LLToolDragAndDrop::getInstance()->dadUpdateInventoryCategory(object,drop);
			break;
		case DAD_TEXTURE:
		case DAD_SOUND:
		case DAD_LANDMARK:
		case DAD_OBJECT:
		case DAD_NOTECARD:
		case DAD_CLOTHING:
		case DAD_BODYPART:
		case DAD_ANIMATION:
		case DAD_GESTURE:
		case DAD_CALLINGCARD:
		case DAD_MESH:
			accept = LLToolDragAndDrop::isInventoryDropAcceptable(object, (LLViewerInventoryItem*)cargo_data);
			if(accept && drop)
			{
				LLToolDragAndDrop::dropInventory(object,
												 (LLViewerInventoryItem*)cargo_data,
												 LLToolDragAndDrop::getInstance()->getSource(),
												 LLToolDragAndDrop::getInstance()->getSourceID());
			}
			break;
		case DAD_SCRIPT:
			// *HACK: In order to resolve SL-22177, we need to block
			// drags from notecards and objects onto other
			// objects. uncomment the simpler version when we have
			// that right.
			//accept = LLToolDragAndDrop::isInventoryDropAcceptable(object, (LLViewerInventoryItem*)cargo_data);
			if(LLToolDragAndDrop::isInventoryDropAcceptable(
				   object, (LLViewerInventoryItem*)cargo_data)
			   && (LLToolDragAndDrop::SOURCE_WORLD != LLToolDragAndDrop::getInstance()->getSource())
			   && (LLToolDragAndDrop::SOURCE_NOTECARD != LLToolDragAndDrop::getInstance()->getSource()))
			{
				accept = TRUE;
			}
			if(accept && drop)
			{
				LLViewerInventoryItem* item = (LLViewerInventoryItem*)cargo_data;
				// rez in the script active by default, rez in
				// inactive if the control key is being held down.
				BOOL active = ((mask & MASK_CONTROL) == 0);
				LLToolDragAndDrop::dropScript(object, item, active,
											  LLToolDragAndDrop::getInstance()->getSource(),
											  LLToolDragAndDrop::getInstance()->getSourceID());
			}
			break;
		default:
			break;
		}
	}
	return accept;
}

///----------------------------------------------------------------------------
/// Class LLTaskTextureBridge
///----------------------------------------------------------------------------

class LLTaskTextureBridge : public LLTaskInvFVBridge
{
public:
	LLTaskTextureBridge(LLPanelObjectInventory* panel,
						const LLUUID& uuid,
						const std::string& name) :
		LLTaskInvFVBridge(panel, uuid, name) {}

	virtual BOOL canOpenItem() const { return TRUE; }
	virtual void openItem();
};

void LLTaskTextureBridge::openItem()
{
	LL_INFOS() << "LLTaskTextureBridge::openItem()" << LL_ENDL;
	LLPreviewTexture* preview = LLFloaterReg::showTypedInstance<LLPreviewTexture>("preview_texture", LLSD(mUUID), TAKE_FOCUS_YES);
	if(preview)
	{
		preview->setObjectID(mPanel->getTaskUUID());
	}
}


///----------------------------------------------------------------------------
/// Class LLTaskSoundBridge
///----------------------------------------------------------------------------

class LLTaskSoundBridge : public LLTaskInvFVBridge
{
public:
	LLTaskSoundBridge(LLPanelObjectInventory* panel,
					  const LLUUID& uuid,
					  const std::string& name) :
		LLTaskInvFVBridge(panel, uuid, name) {}

	virtual BOOL canOpenItem() const { return TRUE; }
	virtual void openItem();
	virtual void performAction(LLInventoryModel* model, std::string action);
	virtual void buildContextMenu(LLMenuGL& menu, U32 flags);
	static void openSoundPreview(void* data);
};

void LLTaskSoundBridge::openItem()
{
	openSoundPreview((void*)this);
}

void LLTaskSoundBridge::openSoundPreview(void* data)
{
	LLTaskSoundBridge* self = (LLTaskSoundBridge*)data;
	if(!self)
		return;

	LLPreviewSound* preview = LLFloaterReg::showTypedInstance<LLPreviewSound>("preview_sound", LLSD(self->mUUID), TAKE_FOCUS_YES);
	if (preview)
	{
		preview->setObjectID(self->mPanel->getTaskUUID());
	}
}

// virtual
void LLTaskSoundBridge::performAction(LLInventoryModel* model, std::string action)
{
	if (action == "task_play")
	{
		LLInventoryItem* item = findItem();
		if(item)
		{
			send_sound_trigger(item->getAssetUUID(), 1.0);
		}
	}
	LLTaskInvFVBridge::performAction(model, action);
}

void LLTaskSoundBridge::buildContextMenu(LLMenuGL& menu, U32 flags)
{
	LLInventoryItem* item = findItem();
	if(!item) return;
	std::vector<std::string> items;
	std::vector<std::string> disabled_items;

	if(item->getPermissions().getOwner() != gAgent.getID()
	   && item->getSaleInfo().isForSale())
	{
		items.push_back(std::string("Task Buy"));

		std::string label= LLTrans::getString("Buy");
		// Check the price of the item.
		S32 price = getPrice();
		if (-1 == price)
		{
			LL_WARNS() << "label_buy_task_bridged_item: Invalid price" << LL_ENDL;
		}
		else
		{
			std::ostringstream info;
// <FS:AW opensim currency support>
//			info <<  LLTrans::getString("BuyforL$") << price;
			info <<  Tea::wrapCurrency(LLTrans::getString("BuyforL$")) << price;
// </FS:AW opensim currency support>
			label.assign(info.str());
		}

		const LLView::child_list_t *list = menu.getChildList();
		LLView::child_list_t::const_iterator itor;
		for (itor = list->begin(); itor != list->end(); ++itor)
		{
			std::string name = (*itor)->getName();
			LLMenuItemCallGL* menu_itemp = dynamic_cast<LLMenuItemCallGL*>(*itor);
			if (name == "Task Buy" && menu_itemp)
			{
				menu_itemp->setLabel(label);
			}
		}
	}
	else if (canOpenItem())
	{
		if (!isItemCopyable())
		{
			disabled_items.push_back(std::string("Task Open"));
		}
	}
	items.push_back(std::string("Task Properties"));
	if(isItemRenameable())
	{
		items.push_back(std::string("Task Rename"));
	}
	if(isItemRemovable())
	{
		items.push_back(std::string("Task Remove"));
	}

	items.push_back(std::string("Task Play"));


	hide_context_entries(menu, items, disabled_items);
}

///----------------------------------------------------------------------------
/// Class LLTaskLandmarkBridge
///----------------------------------------------------------------------------

class LLTaskLandmarkBridge : public LLTaskInvFVBridge
{
public:
	LLTaskLandmarkBridge(LLPanelObjectInventory* panel,
						 const LLUUID& uuid,
						 const std::string& name) :
		LLTaskInvFVBridge(panel, uuid, name) {}
};

///----------------------------------------------------------------------------
/// Class LLTaskCallingCardBridge
///----------------------------------------------------------------------------

class LLTaskCallingCardBridge : public LLTaskInvFVBridge
{
public:
	LLTaskCallingCardBridge(LLPanelObjectInventory* panel,
							const LLUUID& uuid,
							const std::string& name) :
		LLTaskInvFVBridge(panel, uuid, name) {}

	virtual BOOL isItemRenameable() const;
	virtual BOOL renameItem(const std::string& new_name);
};

BOOL LLTaskCallingCardBridge::isItemRenameable() const
{
	return FALSE;
}

BOOL LLTaskCallingCardBridge::renameItem(const std::string& new_name)
{
	return FALSE;
}


///----------------------------------------------------------------------------
/// Class LLTaskScriptBridge
///----------------------------------------------------------------------------

class LLTaskScriptBridge : public LLTaskInvFVBridge
{
public:
	LLTaskScriptBridge(LLPanelObjectInventory* panel,
					   const LLUUID& uuid,
					   const std::string& name) :
		LLTaskInvFVBridge(panel, uuid, name) {}

	//static BOOL enableIfCopyable( void* userdata );
};

class LLTaskLSLBridge : public LLTaskScriptBridge
{
public:
	LLTaskLSLBridge(LLPanelObjectInventory* panel,
					const LLUUID& uuid,
					const std::string& name) :
		LLTaskScriptBridge(panel, uuid, name) {}

	virtual BOOL canOpenItem() const { return TRUE; }
	virtual void openItem();
	virtual BOOL removeItem();
	//virtual void buildContextMenu(LLMenuGL& menu);

	//static void copyToInventory(void* userdata);
};

void LLTaskLSLBridge::openItem()
{
	LL_INFOS() << "LLTaskLSLBridge::openItem() " << mUUID << LL_ENDL;
	LLViewerObject* object = gObjectList.findObject(mPanel->getTaskUUID());
	if(!object || object->isInventoryPending())
	{
		return;
	}
// [RLVa:KB] - Checked: 2010-03-27 (RLVa-1.2.0b) | Modified: RLVa-1.1.0a
	if ( (rlv_handler_t::isEnabled()) && (gRlvAttachmentLocks.isLockedAttachment(object->getRootEdit())) )
	{
		RlvUtil::notifyBlockedViewXXX(LLAssetType::AT_SCRIPT);
		return;
	}
// [/RLVa:KB]
	if (object->permModify() || gAgent.isGodlike())
	{
		// <FS:Ansariel> FIRE-511 / VWR-27512: Can't open script editors from objects individually
		//LLLiveLSLEditor* preview = LLFloaterReg::showTypedInstance<LLLiveLSLEditor>("preview_scriptedit", LLSD(mUUID), TAKE_FOCUS_YES);
		LLLiveLSLEditor* preview = LLFloaterReg::showTypedInstance<LLLiveLSLEditor>("preview_scriptedit", LLSD().with("xoredid", mUUID ^ mPanel->getTaskUUID()).with("assetid", mUUID), TAKE_FOCUS_YES);
		// </FS:Ansariel>
		if (preview)
		{
			preview->setObjectID(mPanel->getTaskUUID());
		}
	}
	else
	{	
		LLNotificationsUtil::add("CannotOpenScriptObjectNoMod");
	}
}

BOOL LLTaskLSLBridge::removeItem()
{
	LLFloaterReg::hideInstance("preview_scriptedit", LLSD(mUUID));
	return LLTaskInvFVBridge::removeItem();
}

///----------------------------------------------------------------------------
/// Class LLTaskObjectBridge
///----------------------------------------------------------------------------

class LLTaskObjectBridge : public LLTaskInvFVBridge
{
public:
	LLTaskObjectBridge(LLPanelObjectInventory* panel,
					   const LLUUID& uuid,
					   const std::string& name,
					   U32 flags = 0) :
		LLTaskInvFVBridge(panel, uuid, name, flags) {}
};

///----------------------------------------------------------------------------
/// Class LLTaskNotecardBridge
///----------------------------------------------------------------------------

class LLTaskNotecardBridge : public LLTaskInvFVBridge
{
public:
	LLTaskNotecardBridge(LLPanelObjectInventory* panel,
						 const LLUUID& uuid,
						 const std::string& name) :
		LLTaskInvFVBridge(panel, uuid, name) {}

	virtual BOOL canOpenItem() const { return TRUE; }
	virtual void openItem();
	virtual BOOL removeItem();
};

void LLTaskNotecardBridge::openItem()
{
	LLViewerObject* object = gObjectList.findObject(mPanel->getTaskUUID());
	if(!object || object->isInventoryPending())
	{
		return;
	}
// [RLVa:KB] - Checked: 2010-03-27 (RLVa-1.2.0b) | Modified: RLVa-1.2.0b
	if ( (rlv_handler_t::isEnabled()) && (gRlvAttachmentLocks.isLockedAttachment(object->getRootEdit())) )
	{
		RlvUtil::notifyBlockedViewXXX(LLAssetType::AT_NOTECARD);
		return;
	}
// [/RLVa:KB]
	if(object->permModify() || gAgent.isGodlike())
	{
		LLPreviewNotecard* preview = LLFloaterReg::showTypedInstance<LLPreviewNotecard>("preview_notecard", LLSD(mUUID), TAKE_FOCUS_YES);
		if (preview)
		{
			preview->setObjectID(mPanel->getTaskUUID());
		}
	}
}

BOOL LLTaskNotecardBridge::removeItem()
{
	LLFloaterReg::hideInstance("preview_notecard", LLSD(mUUID));
	return LLTaskInvFVBridge::removeItem();
}

///----------------------------------------------------------------------------
/// Class LLTaskGestureBridge
///----------------------------------------------------------------------------

class LLTaskGestureBridge : public LLTaskInvFVBridge
{
public:
	LLTaskGestureBridge(LLPanelObjectInventory* panel,
						const LLUUID& uuid,
						const std::string& name) :
	LLTaskInvFVBridge(panel, uuid, name) {}

	virtual BOOL canOpenItem() const { return TRUE; }
	virtual void openItem();
	virtual BOOL removeItem();
};

void LLTaskGestureBridge::openItem()
{
	LLViewerObject* object = gObjectList.findObject(mPanel->getTaskUUID());
	if(!object || object->isInventoryPending())
	{
		return;
	}
	LLPreviewGesture::show(mUUID, mPanel->getTaskUUID());
}

BOOL LLTaskGestureBridge::removeItem()
{
	// Don't need to deactivate gesture because gestures inside objects can never be active.
	LLFloaterReg::hideInstance("preview_gesture", LLSD(mUUID));
	return LLTaskInvFVBridge::removeItem();
}

///----------------------------------------------------------------------------
/// Class LLTaskAnimationBridge
///----------------------------------------------------------------------------

class LLTaskAnimationBridge : public LLTaskInvFVBridge
{
public:
	LLTaskAnimationBridge(LLPanelObjectInventory* panel,
						  const LLUUID& uuid,
						  const std::string& name) :
		LLTaskInvFVBridge(panel, uuid, name) {}

	virtual BOOL canOpenItem() const { return TRUE; }
	virtual void openItem();
	virtual BOOL removeItem();
};

void LLTaskAnimationBridge::openItem()
{
	LLViewerObject* object = gObjectList.findObject(mPanel->getTaskUUID());
	if(!object || object->isInventoryPending())
	{
		return;
	}

	LLPreviewAnim* preview = LLFloaterReg::showTypedInstance<LLPreviewAnim>("preview_anim", LLSD(mUUID), TAKE_FOCUS_YES);
	if (preview && (object->permModify() || gAgent.isGodlike()))
	{
		preview->setObjectID(mPanel->getTaskUUID());
	}
}

BOOL LLTaskAnimationBridge::removeItem()
{
	LLFloaterReg::hideInstance("preview_anim", LLSD(mUUID));
	return LLTaskInvFVBridge::removeItem();
}

///----------------------------------------------------------------------------
/// Class LLTaskWearableBridge
///----------------------------------------------------------------------------

class LLTaskWearableBridge : public LLTaskInvFVBridge
{
public:
	LLTaskWearableBridge(LLPanelObjectInventory* panel,
						 const LLUUID& uuid,
						 const std::string& name,
						 U32 flags) :
		LLTaskInvFVBridge(panel, uuid, name, flags) {}

	virtual LLUIImagePtr getIcon() const;
};

LLUIImagePtr LLTaskWearableBridge::getIcon() const
{
	return LLInventoryIcon::getIcon(mAssetType, mInventoryType, mFlags, FALSE );
}

///----------------------------------------------------------------------------
/// Class LLTaskMeshBridge
///----------------------------------------------------------------------------

class LLTaskMeshBridge : public LLTaskInvFVBridge
{
public:
	LLTaskMeshBridge(
		LLPanelObjectInventory* panel,
		const LLUUID& uuid,
		const std::string& name);

	virtual LLUIImagePtr getIcon() const;
	virtual void openItem();
	virtual void performAction(LLInventoryModel* model, std::string action);
	virtual void buildContextMenu(LLMenuGL& menu, U32 flags);
};

LLTaskMeshBridge::LLTaskMeshBridge(
	LLPanelObjectInventory* panel,
	const LLUUID& uuid,
	const std::string& name) :
	LLTaskInvFVBridge(panel, uuid, name)
{
}

LLUIImagePtr LLTaskMeshBridge::getIcon() const
{
	return LLInventoryIcon::getIcon(LLAssetType::AT_MESH, LLInventoryType::IT_MESH, 0, FALSE);
}

void LLTaskMeshBridge::openItem()
{
	// open mesh
}


// virtual
void LLTaskMeshBridge::performAction(LLInventoryModel* model, std::string action)
{
	if (action == "mesh action")
	{
		LLInventoryItem* item = findItem();
		if(item)
		{
			// do action
		}
	}
	LLTaskInvFVBridge::performAction(model, action);
}

void LLTaskMeshBridge::buildContextMenu(LLMenuGL& menu, U32 flags)
{
	LLInventoryItem* item = findItem();
	if(!item) return;
	std::vector<std::string> items;
	std::vector<std::string> disabled_items;

	if(item->getPermissions().getOwner() != gAgent.getID()
	   && item->getSaleInfo().isForSale())
	{
		items.push_back(std::string("Task Buy"));

		std::string label= LLTrans::getString("Buy");
		// Check the price of the item.
		S32 price = getPrice();
		if (-1 == price)
		{
			LL_WARNS() << "label_buy_task_bridged_item: Invalid price" << LL_ENDL;
		}
		else
		{
			std::ostringstream info;
// <FS:AW opensim currency support>
// 			info <<  LLTrans::getString("BuyforL$") << price;
			info <<  Tea::wrapCurrency(LLTrans::getString("BuyforL$")) << price;
// </FS:AW opensim currency support>
			label.assign(info.str());
		}

		const LLView::child_list_t *list = menu.getChildList();
		LLView::child_list_t::const_iterator itor;
		for (itor = list->begin(); itor != list->end(); ++itor)
		{
			std::string name = (*itor)->getName();
			LLMenuItemCallGL* menu_itemp = dynamic_cast<LLMenuItemCallGL*>(*itor);
			if (name == "Task Buy" && menu_itemp)
			{
				menu_itemp->setLabel(label);
			}
		}
	}
	else
	{
		items.push_back(std::string("Task Open")); 
		if (!isItemCopyable())
		{
			disabled_items.push_back(std::string("Task Open"));
		}
	}
	items.push_back(std::string("Task Properties"));
	if(isItemRenameable())
	{
		items.push_back(std::string("Task Rename"));
	}
	if(isItemRemovable())
	{
		items.push_back(std::string("Task Remove"));
	}


	hide_context_entries(menu, items, disabled_items);
}

///----------------------------------------------------------------------------
/// LLTaskInvFVBridge impl
//----------------------------------------------------------------------------

LLTaskInvFVBridge* LLTaskInvFVBridge::createObjectBridge(LLPanelObjectInventory* panel,
														 LLInventoryObject* object)
{
	LLTaskInvFVBridge* new_bridge = NULL;
	const LLInventoryItem* item = dynamic_cast<LLInventoryItem*>(object);
	const U32 itemflags = ( NULL == item ? 0 : item->getFlags() );
	LLAssetType::EType type = object ? object->getType() : LLAssetType::AT_CATEGORY;
	LLUUID object_id = object ? object->getUUID() : LLUUID::null;
	std::string object_name = object ? object->getName() : std::string();

	switch(type)
	{
	case LLAssetType::AT_TEXTURE:
		new_bridge = new LLTaskTextureBridge(panel,
						     object_id,
						     object_name);
		break;
	case LLAssetType::AT_SOUND:
		new_bridge = new LLTaskSoundBridge(panel,
						   object_id,
						   object_name);
		break;
	case LLAssetType::AT_LANDMARK:
		new_bridge = new LLTaskLandmarkBridge(panel,
						      object_id,
						      object_name);
		break;
	case LLAssetType::AT_CALLINGCARD:
		new_bridge = new LLTaskCallingCardBridge(panel,
							 object_id,
							 object_name);
		break;
	case LLAssetType::AT_SCRIPT:
		// OLD SCRIPTS DEPRECATED - JC
		LL_WARNS() << "Old script" << LL_ENDL;
		//new_bridge = new LLTaskOldScriptBridge(panel,
		//									   object_id,
		//									   object_name);
		break;
	case LLAssetType::AT_OBJECT:
		new_bridge = new LLTaskObjectBridge(panel,
						    object_id,
						    object_name,
						    itemflags);
		break;
	case LLAssetType::AT_NOTECARD:
		new_bridge = new LLTaskNotecardBridge(panel,
						      object_id,
						      object_name);
		break;
	case LLAssetType::AT_ANIMATION:
		new_bridge = new LLTaskAnimationBridge(panel,
						       object_id,
						       object_name);
		break;
	case LLAssetType::AT_GESTURE:
		new_bridge = new LLTaskGestureBridge(panel,
						     object_id,
						     object_name);
		break;
	case LLAssetType::AT_CLOTHING:
	case LLAssetType::AT_BODYPART:
		new_bridge = new LLTaskWearableBridge(panel,
						      object_id,
						      object_name,
						      itemflags);
		break;
	case LLAssetType::AT_CATEGORY:
		new_bridge = new LLTaskCategoryBridge(panel,
						      object_id,
						      object_name);
		break;
	case LLAssetType::AT_LSL_TEXT:
		new_bridge = new LLTaskLSLBridge(panel,
						 object_id,
						 object_name);
		break;
	case LLAssetType::AT_MESH:
		new_bridge = new LLTaskMeshBridge(panel,
										  object_id,
										  object_name);
		break;
	default:
		LL_INFOS() << "Unhandled inventory type (llassetstorage.h): "
				<< (S32)type << LL_ENDL;
		break;
	}
	return new_bridge;
}


///----------------------------------------------------------------------------
/// Class LLPanelObjectInventory
///----------------------------------------------------------------------------

static LLDefaultChildRegistry::Register<LLPanelObjectInventory> r("panel_inventory_object");

void do_nothing()
{
}

// Default constructor
LLPanelObjectInventory::LLPanelObjectInventory(const LLPanelObjectInventory::Params& p) :
	LLPanel(p),
	mScroller(NULL),
	mFolders(NULL),
	mHaveInventory(FALSE),
	mIsInventoryEmpty(TRUE),
	mInventoryNeedsUpdate(FALSE),
	mInventoryViewModel(p.name)
{
	// Setup context menu callbacks
	mCommitCallbackRegistrar.add("Inventory.DoToSelected", boost::bind(&LLPanelObjectInventory::doToSelected, this, _2));
	mCommitCallbackRegistrar.add("Inventory.EmptyTrash", boost::bind(&LLInventoryModel::emptyFolderType, &gInventory, "ConfirmEmptyTrash", LLFolderType::FT_TRASH));
	mCommitCallbackRegistrar.add("Inventory.EmptyLostAndFound", boost::bind(&LLInventoryModel::emptyFolderType, &gInventory, "ConfirmEmptyLostAndFound", LLFolderType::FT_LOST_AND_FOUND));
	mCommitCallbackRegistrar.add("Inventory.DoCreate", boost::bind(&do_nothing));
	mCommitCallbackRegistrar.add("Inventory.AttachObject", boost::bind(&do_nothing));
	mCommitCallbackRegistrar.add("Inventory.BeginIMSession", boost::bind(&do_nothing));
	mCommitCallbackRegistrar.add("Inventory.Share",  boost::bind(&LLAvatarActions::shareWithAvatars, this));
}

// Destroys the object
LLPanelObjectInventory::~LLPanelObjectInventory()
{
	if (!gIdleCallbacks.deleteFunction(idle, this))
	{
		LL_WARNS() << "LLPanelObjectInventory::~LLPanelObjectInventory() failed to delete callback" << LL_ENDL;
	}
}

BOOL LLPanelObjectInventory::postBuild()
{
	// clear contents and initialize menus, sets up mFolders
	reset();

	// Register an idle update callback
	gIdleCallbacks.addFunction(idle, this);

	return TRUE;
}

void LLPanelObjectInventory::doToSelected(const LLSD& userdata)
{
	LLInventoryAction::doToSelected(&gInventory, mFolders, userdata.asString());
}

void LLPanelObjectInventory::clearContents()
{
	mHaveInventory = FALSE;
	mIsInventoryEmpty = TRUE;
	if (LLToolDragAndDrop::getInstance() && LLToolDragAndDrop::getInstance()->getSource() == LLToolDragAndDrop::SOURCE_WORLD)
	{
		LLToolDragAndDrop::getInstance()->endDrag();
	}

	clearItemIDs();

	if( mScroller )
	{
		// removes mFolders
		removeChild( mScroller ); //*TODO: Really shouldn't do this during draw()/refresh()
		mScroller->die();
		mScroller = NULL;
		mFolders = NULL;
	}
}


void LLPanelObjectInventory::reset()
{
	clearContents();

	mCommitCallbackRegistrar.pushScope(); // push local callbacks
	
	// Reset the inventory model to show all folders by default
	mInventoryViewModel.getFilter().setShowFolderState(LLInventoryFilter::SHOW_ALL_FOLDERS);
	
	// Create a new folder view root
	LLRect dummy_rect(0, 1, 1, 0);
	LLFolderView::Params p;
	p.name = "task inventory";
	p.title = "task inventory";
	p.parent_panel = this;
	p.tool_tip= LLTrans::getString("PanelContentsTooltip");
	p.listener = LLTaskInvFVBridge::createObjectBridge(this, NULL);
	p.folder_indentation = -14; // subtract space normally reserved for folder expanders
	p.view_model = &mInventoryViewModel;
	p.root = NULL;
    p.options_menu = "menu_inventory.xml";

	// <FS:Ansariel> Inventory specials
	p.for_inventory = true;

	mFolders = LLUICtrlFactory::create<LLFolderView>(p);

	mFolders->setCallbackRegistrar(&mCommitCallbackRegistrar);

	if (hasFocus())
	{
		LLEditMenuHandler::gEditMenuHandler = mFolders;
	}

	LLRect scroller_rect(0, getRect().getHeight(), getRect().getWidth(), 0);
	LLScrollContainer::Params scroll_p;
	scroll_p.name("task inventory scroller");
	scroll_p.rect(scroller_rect);
	scroll_p.tab_stop(true);
	scroll_p.follows.flags(FOLLOWS_ALL);
	mScroller = LLUICtrlFactory::create<LLFolderViewScrollContainer>(scroll_p);
	addChild(mScroller);
	mScroller->addChild(mFolders);
	
	mFolders->setScrollContainer( mScroller );
	
	mCommitCallbackRegistrar.popScope();
}

void LLPanelObjectInventory::inventoryChanged(LLViewerObject* object,
										LLInventoryObject::object_list_t* inventory,
										S32 serial_num,
										void* data)
{
	if(!object) return;

	//LL_INFOS() << "invetnory arrived: \n"
	//		<< " panel UUID: " << panel->mTaskUUID << "\n"
	//		<< " task  UUID: " << object->mID << LL_ENDL;
	if(mTaskUUID == object->mID)
	{
		mInventoryNeedsUpdate = TRUE;
	}

	// refresh any properties floaters that are hanging around.
	if(inventory)
	{
		for (LLInventoryObject::object_list_t::const_iterator iter = inventory->begin();
			 iter != inventory->end(); )
		{
			LLInventoryObject* item = *iter++;
			LLFloaterProperties* floater = LLFloaterReg::findTypedInstance<LLFloaterProperties>("properties", item->getUUID());
			if(floater)
			{
				floater->refresh();
			}
		}
	}
}

void LLPanelObjectInventory::updateInventory()
{
	//LL_INFOS() << "inventory arrived: \n"
	//		<< " panel UUID: " << panel->mTaskUUID << "\n"
	//		<< " task  UUID: " << object->mID << LL_ENDL;
	// We're still interested in this task's inventory.
	std::vector<LLUUID> selected_item_ids;
	std::set<LLFolderViewItem*> selected_items;
	BOOL inventory_has_focus = FALSE;
	if (mHaveInventory && mFolders)
	{
		selected_items = mFolders->getSelectionList();
		inventory_has_focus = gFocusMgr.childHasKeyboardFocus(mFolders);
	}
	for (std::set<LLFolderViewItem*>::iterator it = selected_items.begin(), end_it = selected_items.end();
		it != end_it;
		++it)
	{
		selected_item_ids.push_back(static_cast<LLFolderViewModelItemInventory*>((*it)->getViewModelItem())->getUUID());
	}

	LLViewerObject* objectp = gObjectList.findObject(mTaskUUID);
	if (objectp)
	{
		LLInventoryObject* inventory_root = objectp->getInventoryRoot();
		LLInventoryObject::object_list_t contents;
		objectp->getInventoryContents(contents);

		if (inventory_root)
		{
			reset();
			mIsInventoryEmpty = FALSE;
			createFolderViews(inventory_root, contents);
			mFolders->setEnabled(TRUE);
		}
		else
		{
			// TODO: create an empty inventory
			mIsInventoryEmpty = TRUE;
		}

		mHaveInventory = TRUE;
	}
	else
	{
		// TODO: create an empty inventory
		mIsInventoryEmpty = TRUE;
		mHaveInventory = TRUE;
	}

	// restore previous selection
	std::vector<LLUUID>::iterator selection_it;
	bool first_item = true;
	for (selection_it = selected_item_ids.begin(); selection_it != selected_item_ids.end(); ++selection_it)
	{
		LLFolderViewItem* selected_item = getItemByID(*selection_it);
		
		if (selected_item)
		{
			//HACK: "set" first item then "change" each other one to get keyboard focus right
			if (first_item)
			{
				mFolders->setSelection(selected_item, TRUE, inventory_has_focus);
				first_item = FALSE;
			}
			else
			{
				mFolders->changeSelection(selected_item, TRUE);
			}
		}
	}

	if (mFolders)
	{
		mFolders->requestArrange();
	}
	mInventoryNeedsUpdate = FALSE;
	// Edit menu handler is set in onFocusReceived
}

// *FIX: This is currently a very expensive operation, because we have
// to iterate through the inventory one time for each category. This
// leads to an N^2 based on the category count. This could be greatly
// speeded with an efficient multimap implementation, but we don't
// have that in our current arsenal.
void LLPanelObjectInventory::createFolderViews(LLInventoryObject* inventory_root, LLInventoryObject::object_list_t& contents)
{
	if (!inventory_root)
	{
		return;
	}
	// Create a visible root category.
	LLTaskInvFVBridge* bridge = NULL;
	bridge = LLTaskInvFVBridge::createObjectBridge(this, inventory_root);
	if(bridge)
	{
		LLUIColor item_color = LLUIColorTable::instance().getColor("MenuItemEnabledColor", DEFAULT_WHITE);

		LLFolderViewFolder::Params p;
		p.name = inventory_root->getName();
		p.tool_tip = p.name;
		p.root = mFolders;
		p.listener = bridge;
		p.font_color = item_color;
		p.font_highlight_color = item_color;

		// <FS:Ansariel> Inventory specials
		p.for_inventory = true;

		LLFolderViewFolder* new_folder = LLUICtrlFactory::create<LLFolderViewFolder>(p);
		new_folder->addToFolder(mFolders);
		new_folder->toggleOpen();

		if (!contents.empty())
		{
			createViewsForCategory(&contents, inventory_root, new_folder);
		}
	}
}

typedef std::pair<LLInventoryObject*, LLFolderViewFolder*> obj_folder_pair;

void LLPanelObjectInventory::createViewsForCategory(LLInventoryObject::object_list_t* inventory, 
											  LLInventoryObject* parent,
											  LLFolderViewFolder* folder)
{
	LLUIColor item_color = LLUIColorTable::instance().getColor("MenuItemEnabledColor", DEFAULT_WHITE);

	// Find all in the first pass
	std::vector<obj_folder_pair*> child_categories;
	LLTaskInvFVBridge* bridge;
	LLFolderViewItem* view;

	LLInventoryObject::object_list_t::iterator it = inventory->begin();
	LLInventoryObject::object_list_t::iterator end = inventory->end();
	for( ; it != end; ++it)
	{
		LLInventoryObject* obj = *it;

		if(parent->getUUID() == obj->getParentUUID())
		{
			bridge = LLTaskInvFVBridge::createObjectBridge(this, obj);
			if(!bridge)
			{
				continue;
			}
			if(LLAssetType::AT_CATEGORY == obj->getType())
			{
				LLFolderViewFolder::Params p;
				p.name = obj->getName();
				p.root = mFolders;
				p.listener = bridge;
				p.tool_tip = p.name;
				p.font_color = item_color;
				p.font_highlight_color = item_color;

				// <FS:Ansariel> Inventory specials
				p.for_inventory = true;

				view = LLUICtrlFactory::create<LLFolderViewFolder>(p);
				child_categories.push_back(new obj_folder_pair(obj,
														 (LLFolderViewFolder*)view));
			}
			else
			{
				LLFolderViewItem::Params params;
				params.name(obj->getName());
				params.creation_date(bridge->getCreationDate());
				params.root(mFolders);
				params.listener(bridge);
				params.rect(LLRect());
				params.tool_tip = params.name;
				params.font_color = item_color;
				params.font_highlight_color = item_color;

				// <FS:Ansariel> Inventory specials
				params.for_inventory = true;

				view = LLUICtrlFactory::create<LLFolderViewItem> (params);
			}
			view->addToFolder(folder);
			addItemID(obj->getUUID(), view);
		}
	}

	// now, for each category, do the second pass
	for(S32 i = 0; i < child_categories.size(); i++)
	{
		createViewsForCategory(inventory, child_categories[i]->first,
							   child_categories[i]->second );
		delete child_categories[i];
	}
}

void LLPanelObjectInventory::refresh()
{
	//LL_INFOS() << "LLPanelObjectInventory::refresh()" << LL_ENDL;
	BOOL has_inventory = FALSE;
	const BOOL non_root_ok = TRUE;
	LLSelectNode* node = LLSelectMgr::getInstance()->getSelection()->getFirstRootNode(NULL, non_root_ok);
	if(node)
	{
		LLViewerObject* object = node->getObject();
		if(object && ((LLSelectMgr::getInstance()->getSelection()->getRootObjectCount() == 1)
					  || (LLSelectMgr::getInstance()->getSelection()->getObjectCount() == 1)))
		{
			// determine if we need to make a request. Start with a
			// default based on if we have inventory at all.
			BOOL make_request = !mHaveInventory;

			// If the task id is different than what we've stored,
			// then make the request.
			if(mTaskUUID != object->mID)
			{
				mTaskUUID = object->mID;
				make_request = TRUE;

				// This is a new object so pre-emptively clear the contents
				// Otherwise we show the old stuff until the update comes in
				clearContents();

				// Register for updates from this object,
				registerVOInventoryListener(object,NULL);
			}

			// Based on the node information, we may need to dirty the
			// object inventory and get it again.
			if(node->mValid)
			{
				if(node->mInventorySerial != object->getInventorySerial() || object->isInventoryDirty())
				{
					make_request = TRUE;
				}
			}

			// do the request if necessary.
			if(make_request)
			{
				requestVOInventory();
			}
			has_inventory = TRUE;
		}
	}
	if(!has_inventory)
	{
		clearInventoryTask();
	}
	mInventoryViewModel.setTaskID(mTaskUUID);
	//LL_INFOS() << "LLPanelObjectInventory::refresh() " << mTaskUUID << LL_ENDL;
}

void LLPanelObjectInventory::clearInventoryTask()
{
	mTaskUUID = LLUUID::null;
	removeVOInventoryListener();
	clearContents();
}

void LLPanelObjectInventory::removeSelectedItem()
{
	if(mFolders)
	{
		mFolders->removeSelectedItems();
	}
}

void LLPanelObjectInventory::startRenamingSelectedItem()
{
	if(mFolders)
	{
		mFolders->startRenamingSelectedItem();
	}
}

void LLPanelObjectInventory::draw()
{
	LLPanel::draw();

	if(mIsInventoryEmpty)
	{
		if((LLUUID::null != mTaskUUID) && (!mHaveInventory))
		{
			LLFontGL::getFontSansSerif()->renderUTF8(LLTrans::getString("LoadingContents"), 0,
													 (S32)(getRect().getWidth() * 0.5f),
													 10,
													 LLColor4( 1, 1, 1, 1 ),
													 LLFontGL::HCENTER,
													 LLFontGL::BOTTOM);
		}
		else if(mHaveInventory)
		{
			LLFontGL::getFontSansSerif()->renderUTF8(LLTrans::getString("NoContents"), 0,
													 (S32)(getRect().getWidth() * 0.5f),
													 10,
													 LLColor4( 1, 1, 1, 1 ),
													 LLFontGL::HCENTER,
													 LLFontGL::BOTTOM);
		}
	}
}

void LLPanelObjectInventory::deleteAllChildren()
{
	mScroller = NULL;
	mFolders = NULL;
	LLView::deleteAllChildren();
}

BOOL LLPanelObjectInventory::handleDragAndDrop(S32 x, S32 y, MASK mask, BOOL drop, EDragAndDropType cargo_type, void *cargo_data, EAcceptance *accept, std::string& tooltip_msg)
{
	if (mFolders && mHaveInventory)
	{
		LLFolderViewItem* folderp = mFolders->getNextFromChild(NULL);
		if (!folderp)
		{
			return FALSE;
		}
		// Try to pass on unmodified mouse coordinates
		S32 local_x = x - mFolders->getRect().mLeft;
		S32 local_y = y - mFolders->getRect().mBottom;

		if (mFolders->pointInView(local_x, local_y))
		{
			return mFolders->handleDragAndDrop(local_x, local_y, mask, drop, cargo_type, cargo_data, accept, tooltip_msg);
		}
		else
		{
			//force mouse coordinates to be inside folder rectangle
			return mFolders->handleDragAndDrop(5, 1, mask, drop, cargo_type, cargo_data, accept, tooltip_msg);
		}
	}
	else
	{
		return FALSE;
	}
}

//static
void LLPanelObjectInventory::idle(void* user_data)
{
	LLPanelObjectInventory* self = (LLPanelObjectInventory*)user_data;

	if (self->mFolders)
	{
		self->mFolders->update();
	}
	if (self->mInventoryNeedsUpdate)
	{
		self->updateInventory();
	}
}

void LLPanelObjectInventory::onFocusLost()
{
	// inventory no longer handles cut/copy/paste/delete
	if (LLEditMenuHandler::gEditMenuHandler == mFolders)
	{
		LLEditMenuHandler::gEditMenuHandler = NULL;
	}
	
	LLPanel::onFocusLost();
}

void LLPanelObjectInventory::onFocusReceived()
{
	// inventory now handles cut/copy/paste/delete
	LLEditMenuHandler::gEditMenuHandler = mFolders;
	
	LLPanel::onFocusReceived();
}


LLFolderViewItem* LLPanelObjectInventory::getItemByID( const LLUUID& id )
{
	std::map<LLUUID, LLFolderViewItem*>::iterator map_it;
	map_it = mItemMap.find(id);
	if (map_it != mItemMap.end())
	{
		return map_it->second;
	}

	return NULL;
}

void LLPanelObjectInventory::removeItemID( const LLUUID& id )
{
	mItemMap.erase(id);
}

void LLPanelObjectInventory::addItemID( const LLUUID& id, LLFolderViewItem* itemp )
{
	mItemMap[id] = itemp;
}

void LLPanelObjectInventory::clearItemIDs()
{
	mItemMap.clear();
}

BOOL LLPanelObjectInventory::handleKeyHere( KEY key, MASK mask )
{
	BOOL handled = FALSE;
	switch (key)
	{
// <FS:Ansariel> Fix broken return key in task inventory
	case KEY_RETURN:
		if (mask == MASK_NONE)
		{
			LLPanelObjectInventory::doToSelected(LLSD("task_open"));
			handled = TRUE;
		}
		break;
// </FS:Ansariel> Fix broken return key in task inventory
	case KEY_DELETE:
	case KEY_BACKSPACE:
		// Delete selected items if delete or backspace key hit on the inventory panel
		// Note: on Mac laptop keyboards, backspace and delete are one and the same
		if (isSelectionRemovable() && mask == MASK_NONE)
		{
			LLInventoryAction::doToSelected(&gInventory, mFolders, "delete");
			handled = TRUE;
		}
		break;
	}
	return handled;
}

BOOL LLPanelObjectInventory::isSelectionRemovable()
{
	if (!mFolders || !mFolders->getRoot())
	{
		return FALSE;
	}
	std::set<LLFolderViewItem*> selection_set = mFolders->getRoot()->getSelectionList();
	if (selection_set.empty())
	{
		return FALSE;
	}
	for (std::set<LLFolderViewItem*>::iterator iter = selection_set.begin();
		iter != selection_set.end();
		++iter)
	{
		LLFolderViewItem *item = *iter;
		const LLFolderViewModelItemInventory *listener = dynamic_cast<const LLFolderViewModelItemInventory*>(item->getViewModelItem());
		if (!listener || !listener->isItemRemovable() || listener->isItemInTrash())
		{
			return FALSE;
		}
	}
	return TRUE;
}
