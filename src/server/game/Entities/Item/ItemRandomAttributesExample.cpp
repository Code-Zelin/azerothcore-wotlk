/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ItemRandomAttributes.h"
#include "Item.h"
#include "Player.h"
#include "Chat.h"
#include "ScriptMgr.h"

// 示例：在物品创建时自动生成随机属性
class ItemRandomAttributesExample : public ItemScript
{
public:
    ItemRandomAttributesExample() : ItemScript("ItemRandomAttributesExample") {}

    void OnCreate(Item* item, Player* player) override
    {
        if (!item || !player)
            return;

        // 根据物品来源设置不同的来源类型
        uint32 sourceType = ITEM_SOURCE_OTHER;
        
        // 这里可以根据实际情况判断物品来源
        // 例如：从怪物掉落、任务奖励、制造等
        
        // 生成随机属性
        if (sItemRandomAttributesMgr->GenerateRandomAttributes(item, sourceType))
        {
            // 获取生成的属性
            std::vector<ItemRandomAttribute> attributes = sItemRandomAttributesMgr->GetItemAttributes(item->GetGUID().GetCounter());
            
            if (!attributes.empty())
            {
                // 向玩家显示生成的属性
                ChatHandler(player->GetSession()).PSendSysMessage("物品获得了 %zu 个随机属性:", attributes.size());
                
                for (const auto& attr : attributes)
                {
                    std::string displayText = sItemRandomAttributesMgr->GetAttributeDisplayText(attr);
                    ChatHandler(player->GetSession()).PSendSysMessage("- %s", displayText.c_str());
                }
            }
        }
    }
};

// 注册脚本
void AddSC_ItemRandomAttributesExample()
{
    new ItemRandomAttributesExample();
} 